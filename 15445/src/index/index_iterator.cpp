/**
 * index_iterator.cpp
 */
#include <cassert>

#include "index/index_iterator.h"

namespace cmudb {

    /*
     * NOTE: you can change the destructor/constructor method here
     * set your own input parameters
     */
    INDEX_TEMPLATE_ARGUMENTS
        INDEXITERATOR_TYPE::IndexIterator() {}
    
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE::IndexIterator(const page_id_t page_id, 
         const page_id_t next_page_id, const int pos, const int size, BufferPoolManager* bpm) {
        page_id_ = page_id;
        next_pg_id_ = next_page_id;
        pos_ = pos;
        size_ = size;
        buffer_pool_manager_ = bpm;
    }

    INDEX_TEMPLATE_ARGUMENTS
        INDEXITERATOR_TYPE::~IndexIterator() {}

    INDEX_TEMPLATE_ARGUMENTS
        bool INDEXITERATOR_TYPE::isEnd() { 
            if(pos_ == size_) {
                if(next_pg_id_ == INVALID_PAGE_ID) {
                    return true;
                }
            }
            return false; 
        }

    INDEX_TEMPLATE_ARGUMENTS
        const MappingType &INDEXITERATOR_TYPE::operator*() {
            // NOTE: replace with your own implementation
            auto pg = buffer_pool_manager_->FetchPage(page_id_);
            auto node =  reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(pg->GetData());
            return node->GetItem(pos_);
        }

    INDEX_TEMPLATE_ARGUMENTS
        INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
            pos_++;
            if(pos_ < size_) {
                return *this;
            } 
            if(next_pg_id_ == INVALID_PAGE_ID)
                return *this;
            // check if has next page
            auto pg = buffer_pool_manager_->FetchPage(next_pg_id_);
            auto node =  reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(pg->GetData());
            auto nextpg = node->GetNextPageId();
            page_id_ = next_pg_id_;
            next_pg_id_ = nextpg;
            pos_ = 0;
            size_ = node->GetSize();
            buffer_pool_manager_->UnpinPage(next_pg_id_, false);
            return *this; 
        }

    template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;
    template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;
    template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;
    template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;
    template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

} // namespace cmudb
