#include "buffer/buffer_pool_manager.h"

#include <type_traits>
#include "common/logger.h"

namespace cmudb {

    /*
     * BufferPoolManager Constructor
     * WARNING: Do Not Edit This Function
     */
    BufferPoolManager::BufferPoolManager(size_t pool_size,
            const std::string &db_file)
        : pool_size_(pool_size) {
            // a consecutive memory space for buffer pool
            disk_manager_ = new DiskManager(db_file);
            pages_ = new Page[pool_size_];
            page_table_ = new ExtendibleHash<page_id_t, Page *>(100);
            replacer_ = new LRUReplacer<Page *>;
            free_list_ = new std::list<Page *>;

            // put all the pages into free list
            for (size_t i = 0; i < pool_size_; ++i) {
                free_list_->push_back(&pages_[i]);
            }
        }

    BufferPoolManager::BufferPoolManager(size_t pool_size,
            DiskManager *disk_manager,
            LogManager *log_manager)
        : pool_size_(pool_size), disk_manager_(disk_manager),
        log_manager_(log_manager) {
            // a consecutive memory space for buffer pool
            pages_ = new Page[pool_size_];
            page_table_ = new ExtendibleHash<page_id_t, Page *>(BUCKET_SIZE);
            replacer_ = new LRUReplacer<Page *>;
            free_list_ = new std::list<Page *>;

            // put all the pages into free list
            for (size_t i = 0; i < pool_size_; ++i) {
                free_list_->push_back(&pages_[i]);
            }
        }
    /*
     * BufferPoolManager Deconstructor
     * WARNING: Do Not Edit This Function
     */
    BufferPoolManager::~BufferPoolManager() {
        FlushAllPages();
        delete[] pages_;
        delete page_table_;
        delete replacer_;
        delete free_list_;
    }

    /**
     * 1. search hash table.
     *  1.1 if exist, pin the page and return immediately
     *  1.2 if no exist, find a replacement entry from either free list or lru
     *      replacer. (NOTE: always find from free list first)
     * 2. If the entry chosen for replacement is dirty, write it back to disk.
     * 3. Delete the entry for the old page from the hash table and insert an entry
     * for the new page.
     * 4. Update page metadata, read page content from disk file and return page
     * pointer
     */
    Page *BufferPoolManager::FetchPage(page_id_t page_id) { 
        Page *pg = nullptr;
        bool exist = page_table_->Find(page_id, pg);
        if(exist) {
            //static_assert(pg!=nullptr, "Find page is nullptr");
            pg->pin_count_++;
            return pg;
        } 
        if(free_list_->size() > 0) {
            pg = free_list_->front();
            free_list_->pop_front();
        } else {
            bool exist = replacer_->Victim(pg);      
            if(exist == false) {
                return nullptr;
            }
        }
        //static_assert(pg!=nullptr, "fetch page is nullptr");
        if(pg->is_dirty_) {
            // write back to disk
            disk_manager_->WritePage(pg->page_id_, pg->GetData());
        }
        // delete entry from table hash
        exist = page_table_->Remove(pg->page_id_);
        if(!exist) {
            LOG_DEBUG("removepage|pageid=%d\n", pg->page_id_);
        }
        pg->page_id_= page_id;
        pg->pin_count_++;
        pg->is_dirty_ = false;
        disk_manager_->ReadPage(page_id, pg->GetData());
        page_table_->Insert(page_id, pg);
        return pg;
    }

    /*
     * Implementation of unpin page
     * if pin_count>0, decrement it and if it becomes zero, put it back to replacer
     * if pin_count<=0 before this call, return false.
     * is_dirty: set the dirty flag of this page
     */
    bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
        Page *pg = nullptr;
        bool exist = page_table_->Find(page_id, pg);
        if(!exist) {
            return false;
        }
        if(pg->pin_count_ <= 0) {
            return false;
        }
        pg->is_dirty_ = is_dirty;
        pg->pin_count_--; 
        if(pg->pin_count_ == 0) {
            //put it back to replacer
            replacer_->Insert(pg);
        }
        return true;
    }

    /*
     * Used to flush a particular page of the buffer pool to disk. Should call the
     * write_page method of the disk manager
     * if page is not found in page table, return false
     * NOTE: make sure page_id != INVALID_PAGE_ID
     */
    bool BufferPoolManager::FlushPage(page_id_t page_id) { 
        if(page_id == INVALID_PAGE_ID) {
            return false;  
        }
        Page *pg = nullptr;
        bool exist = page_table_->Find(page_id, pg);
        if(!exist) {
            return false;
        }
        disk_manager_->WritePage(pg->page_id_, pg->GetData());
        pg->is_dirty_ = false;
        return true; 
    }

    /*
     * Used to flush all dirty pages in the buffer pool manager
     */
    void BufferPoolManager::FlushAllPages() {
        for(size_t i=0; i< pool_size_; ++i) {
            if(pages_[i].is_dirty_) {
                disk_manager_->WritePage(pages_[i].page_id_,
                        pages_[i].GetData());
            }
        }
    }

    /**
     * User should call this method for deleting a page. This routine will call disk
     * manager to deallocate the page.
     * First, if page is found within page table, buffer pool manager should be
     * reponsible for removing this entry out of page table, reseting page metadata
     * and adding back to free list. Second, call disk manager's DeallocatePage()
     * method to delete from disk file.
     * If the page is found within page table, but pin_count != 0, return false
     */
    bool BufferPoolManager::DeletePage(page_id_t page_id) { 
        Page *pg = nullptr;
        bool exist = page_table_->Find(page_id, pg);
        if(exist && pg->pin_count_ > 0) {
            return false;
        }
        if(exist) {
            page_table_->Remove(page_id);
            pg->page_id_ = INVALID_PAGE_ID; 
            pg->pin_count_ = 0;
            pg->is_dirty_ = false;
            free_list_->push_back(pg);
        }
        disk_manager_->DeallocatePage(page_id);
        return false; 
    }

    /**
     * User should call this method if needs to create a new page. This routine
     * will call disk manager to allocate a page.
     * Buffer pool manager should be responsible to choose a victim page either from
     * free list or lru replacer(NOTE: always choose from free list first), update
     * new page's metadata, zero out memory and add corresponding entry into page
     * table.
     * return nullptr is all the pages in pool are pinned
     */
    Page *BufferPoolManager::NewPage(page_id_t &page_id) { 
        page_id = disk_manager_->AllocatePage();
        // get a Page
        Page *pg = nullptr;
        if(free_list_->size() > 0) {
            pg = free_list_->front();
            free_list_->pop_front();
        } else {
            bool exist = replacer_->Victim(pg);      
            if(exist == false) {
                return nullptr;
            }
            FlushPage(pg->page_id_);
            page_table_->Remove(pg->page_id_);
        }
        //static_assert(pg!=nullptr, "new page is nullptr");
        pg->ResetMemory();
        pg->pin_count_++;
        pg->page_id_ = page_id;
        page_table_->Insert(page_id, pg);
        return pg;
    }

} // namespace cmudb
