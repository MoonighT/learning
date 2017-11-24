/**
 * b_plus_tree.cpp
 */
#include <iostream>
#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "index/b_plus_tree.h"
#include "page/b_plus_tree_page.h"
#include "page/b_plus_tree_leaf_page.h"
#include "page/b_plus_tree_internal_page.h"
#include "page/header_page.h"
#include "page/page.h"


namespace cmudb {

    INDEX_TEMPLATE_ARGUMENTS
        BPLUSTREE_TYPE::BPlusTree(const std::string &name,
                BufferPoolManager *buffer_pool_manager,
                const KeyComparator &comparator,
                page_id_t root_page_id)
        : index_name_(name), root_page_id_(root_page_id),
        buffer_pool_manager_(buffer_pool_manager),
        comparator_(comparator), size_(0) {}

    /*
     * Helper function to decide whether current b+tree is empty
     */
    INDEX_TEMPLATE_ARGUMENTS
        bool BPLUSTREE_TYPE::IsEmpty() const { 
            return size_ == 0; 
        }
    /*****************************************************************************
     * SEARCH
     *****************************************************************************/
    /*
     * Return the only value that associated with input key
     * This method is used for point query
     * @return : true means key exists
     */
    INDEX_TEMPLATE_ARGUMENTS
        bool BPLUSTREE_TYPE::GetValue(const KeyType &key,
                std::vector<ValueType> &result,
                Transaction *transaction) {
            auto leafPage = FindLeafPage(key, false);
            ValueType val;
            bool exist = leafPage->Lookup(key, val, comparator_);
            if(exist) {
                result.push_back(val);
            }
            return exist;
        }

    /*****************************************************************************
     * INSERTION
     *****************************************************************************/
    /*
     * Insert constant key & value pair into b+ tree
     * if current tree is empty, start new tree, update root page id and insert
     * entry, otherwise insert into leaf page.
     * @return: since we only support unique key, if user try to insert duplicate
     * keys return false, otherwise return true.
     */
    INDEX_TEMPLATE_ARGUMENTS
        bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value,
                Transaction *transaction) {
            if(IsEmpty()) {
                StartNewTree(key, value);
            } else {
                // find page insert record
                InsertIntoLeaf(key, value, transaction);
            }
            size_++;
            return false;
        }
    /*
     * Insert constant key & value pair into an empty tree
     * User needs to first ask for new page from buffer pool manager(NOTICE: throw
     * an "out of memory" exception if returned value is nullptr), then update b+
     * tree's root page id and insert entry directly into leaf page.
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
            page_id_t pageid;
            auto pg = buffer_pool_manager_->NewPage(pageid);
            if(pg == nullptr) {
                throw Exception(EXCEPTION_TYPE_OUTOFMEMORY,
                                "out of memory");    
            }
            // update root page
            root_page_id_ = pageid; 
            UpdateRootPageId(1);
            // insert into a empty tree
            // set up a leaf page and insert key value
            B_PLUS_TREE_LEAF_PAGE_TYPE* node = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>(pg->GetData());
            node->Init(pageid, INVALID_PAGE_ID);
            node->Insert(key, value, comparator_);
            buffer_pool_manager_->UnpinPage(pageid, true);
            return;
        }

    /*
     * Insert constant key & value pair into leaf page
     * User needs to first find the right leaf page as insertion target, then look
     * through leaf page to see whether insert key exist or not. If exist, return
     * immdiately, otherwise insert entry. Remember to deal with split if necessary.
     * @return: since we only support unique key, if user try to insert duplicate
     * keys return false, otherwise return true.
     */
    INDEX_TEMPLATE_ARGUMENTS
        bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value,
                Transaction *transaction) {
            // find leaf page
            auto node = FindLeafPage(key, false);
            printf("insertleaf|pageid=%d\n", node->GetPageId());
            ValueType val;
            bool exist = node->Lookup(key, val, comparator_);
            if(exist) 
                return false;
            // not exist need to insert
            auto num = node->Insert(key, value, comparator_);
            
            if(num >= node->GetMaxSize()) {
                // need to split
                // newNode is next node
                auto newNode = Split(node);
                // need to insert mid key into parent
                InsertIntoParent(node, key, newNode, transaction);
            }
            return true;
        }

    /*
     * Split input page and return newly created page.
     * Using template N to represent either internal page or leaf page.
     * User needs to first ask for new page from buffer pool manager(NOTICE: throw
     * an "out of memory" exception if returned value is nullptr), then move half
     * of key & value pairs from input page to newly created page
     */
    INDEX_TEMPLATE_ARGUMENTS
        template <typename N> N *BPLUSTREE_TYPE::Split(N *node) { 
            page_id_t pageid;
            auto pg = buffer_pool_manager_->NewPage(pageid);
            if(pg == nullptr) {
                throw Exception(EXCEPTION_TYPE_OUTOFMEMORY,
                                "out of memory");    
            }
            // need to check node is leaf or internal
            auto newNode =  reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(pg->GetData());
            newNode->Init(pageid, node->GetParentPageId());
            // move second half to newnode
            node->MoveHalfTo(newNode, buffer_pool_manager_);
            node->SetNextPageId(pageid);
            buffer_pool_manager_->UnpinPage(pageid, true);
            return newNode;
        }

    /*
     * Insert key & value pair into internal page after split
     * @param   old_node      input page from split() method
     * @param   key
     * @param   new_node      returned page from split() method
     * User needs to first find the parent page of old_node, parent node must be
     * adjusted to take info of new_node into account. Remember to deal with split
     * recursively if necessary.
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node,
                const KeyType &key,
                BPlusTreePage *new_node,
                Transaction *transaction) {
            // need to cast
            KeyType midkey; 
            if(new_node->IsLeafPage()) {
                midkey = static_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>(new_node)->KeyAt(0);
            } else {
                midkey = static_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE*>(new_node)->KeyAt(0);
            }
            // if parent node is nil
            if(old_node->GetParentPageId() == INVALID_PAGE_ID) {
                page_id_t pageid;
                auto pg = buffer_pool_manager_->NewPage(pageid);
                if(pg == nullptr) {
                    throw Exception(EXCEPTION_TYPE_OUTOFMEMORY,
                            "out of memory");    
                }        
                // create parent node as internal node
                auto pnode =  reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE*>(pg->GetData());
                pnode->Init(pageid, INVALID_PAGE_ID);
                // update old node and new node parent
                old_node->SetParentPageId(pageid);
                new_node->SetParentPageId(pageid);
                // populate mid node 
                pnode->PopulateNewRoot(old_node->GetPageId(), midkey, new_node->GetPageId());
                root_page_id_ = pnode->GetPageId();
                printf("update root page id =%d\n", root_page_id_);
                UpdateRootPageId(0);
                buffer_pool_manager_->UnpinPage(pageid, true);
            } else {
                // else just insert new node into parent node
                new_node->SetParentPageId(old_node->GetParentPageId());
                // need to check if node if leaf or internal 
                auto ppageid = old_node->GetParentPageId();
                auto pg = buffer_pool_manager_->FetchPage(ppageid);
                auto pnode =  reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE *>(pg->GetData());
                if(old_node->IsLeafPage()) {
                // if leaf copy first from newnode to parent 
                    pnode->InsertNodeAfter(old_node->GetPageId(),
                             midkey, new_node->GetPageId());
                } else {
                // if internal move first from newnode to parent
                    pnode->InsertNodeAfter(old_node->GetPageId(),
                             midkey, new_node->GetPageId());
                    static_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE*>(new_node)->Remove(0);
                }
                // need to check size and call split again
                buffer_pool_manager_->UnpinPage(ppageid, true);
            }
        }

    /*****************************************************************************
     * REMOVE
     *****************************************************************************/
    /*
     * Delete key & value pair associated with input key
     * If current tree is empty, return immdiately.
     * If not, User needs to first find the right leaf page as deletion target, then
     * delete entry from leaf page. Remember to deal with redistribute or merge if
     * necessary.
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
            if(IsEmpty()) {
                return;
            }
            // auto leafpage = FindLeafPage(key);
            // delete entry from page
            // may need to merge 
            size_--;
        }

    /*
     * User needs to first find the sibling of input page. If sibling's size + input
     * page's size > page's max size, then redistribute. Otherwise, merge.
     * Using template N to represent either internal page or leaf page.
     * @return: true means target leaf page should be deleted, false means no
     * deletion happens
     */
    INDEX_TEMPLATE_ARGUMENTS
        template <typename N>
        bool BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {
            return false;
        }

    /*
     * Move all the key & value pairs from one page to its sibling page, and notify
     * buffer pool manager to delete this page. Parent page must be adjusted to
     * take info of deletion into account. Remember to deal with coalesce or
     * redistribute recursively if necessary.
     * Using template N to represent either internal page or leaf page.
     * @param   neighbor_node      sibling page of input "node"
     * @param   node               input from method coalesceOrRedistribute()
     * @param   parent             parent page of input "node"
     * @return  true means parent node should be deleted, false means no deletion
     * happend
     */
    INDEX_TEMPLATE_ARGUMENTS
        template <typename N>
        bool BPLUSTREE_TYPE::Coalesce(
                N *&neighbor_node, N *&node,
                BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *&parent,
                int index, Transaction *transaction) {
            return false;
        }

    /*
     * Redistribute key & value pairs from one page to its sibling page. If index ==
     * 0, move sibling page's first key & value pair into end of input "node",
     * otherwise move sibling page's last key & value pair into head of input
     * "node".
     * Using template N to represent either internal page or leaf page.
     * @param   neighbor_node      sibling page of input "node"
     * @param   node               input from method coalesceOrRedistribute()
     */
    INDEX_TEMPLATE_ARGUMENTS
        template <typename N>
        void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {}
    /*
     * Update root page if necessary
     * NOTE: size of root page can be less than min size and this method is only
     * called within coalesceOrRedistribute() method
     * case 1: when you delete the last element in root page, but root page still
     * has one last child
     * case 2: when you delete the last element in whole b+ tree
     * @return : true means root page should be deleted, false means no deletion
     * happend
     */
    INDEX_TEMPLATE_ARGUMENTS
        bool BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
            return false;
        }

    /*****************************************************************************
     * INDEX ITERATOR
     *****************************************************************************/
    /*
     * Input parameter is void, find the leaftmost leaf page first, then construct
     * index iterator
     * @return : index iterator
     */
    INDEX_TEMPLATE_ARGUMENTS
        INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() { return INDEXITERATOR_TYPE(); }

    /*
     * Input parameter is low key, find the leaf page that contains the input key
     * first, then construct index iterator
     * @return : index iterator
     */
    INDEX_TEMPLATE_ARGUMENTS
        INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
            return INDEXITERATOR_TYPE();
        }

    /*****************************************************************************
     * UTILITIES AND DEBUG
     *****************************************************************************/
    /*
     * Find leaf page containing particular key, if leftMost flag == true, find
     * the left most leaf page
     */
    INDEX_TEMPLATE_ARGUMENTS
        B_PLUS_TREE_LEAF_PAGE_TYPE *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key,
                bool leftMost) {
            B_PLUS_TREE_LEAF_PAGE_TYPE *result = nullptr;
            if(leftMost) {

            } else {
                auto pg_id = root_page_id_;
                // keep search page with key until meet first leaf page
                while(result==nullptr) {
                    Page* pg = buffer_pool_manager_->FetchPage(pg_id);
                    if (pg == nullptr) {
                        throw Exception(EXCEPTION_TYPE_INDEX,
                                "root page is pinned");
                    }
                    BPlusTreePage *node = reinterpret_cast<BPlusTreePage *>(pg->GetData());
                    if(!node->IsLeafPage()) {
                        auto interPage = 
                            reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE *>(pg->GetData());
                        const page_id_t pid = interPage->Lookup(key, comparator_);
                        pg_id = pid; 
                        printf("findleaf|internalpage, pageid=%d\n", pg_id);
                    } else {
                        auto leafPage = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>(pg->GetData());
                        printf("findleaf|pageid=%d\n", leafPage->GetPageId());
                        result = leafPage;
                    }
                    buffer_pool_manager_->UnpinPage(root_page_id_, false);
                }
            }
            return result;
        }

    /*
     * Update/Insert root page id in header page(where page_id = 0, header_page is
     * defined under include/page/header_page.h)
     * Call this method everytime root page id is changed.
     * @parameter: insert_record      defualt value is false. When set to true,
     * insert a record <index_name, root_page_id> into header page instead of
     * updating it.
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
            HeaderPage *header_page = static_cast<HeaderPage *>(
                    buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
            if (insert_record)
                // create a new record<index_name + root_page_id> in header_page
                header_page->InsertRecord(index_name_, root_page_id_);
            else
                // update root_page_id in header_page
                header_page->UpdateRecord(index_name_, root_page_id_);
            buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
        }


     /* This method is used for debug only
     * print out whole b+tree sturcture, rank by rank
     */
    INDEX_TEMPLATE_ARGUMENTS
        std::string BPLUSTREE_TYPE::ToString(bool verbose) { 
            std::string result;
            std::queue<page_id_t> page_queue;
            page_queue.push(root_page_id_);
            while (!page_queue.empty()) {
                auto page_id = page_queue.front();
                auto pg = buffer_pool_manager_->FetchPage(page_id);
                auto target_page = reinterpret_cast<BPlusTreePage *>(pg->GetData());
                printf("pageid=%d, size=%d\n", page_id, target_page->GetSize());
                if(!target_page->IsLeafPage()) {
                    auto interPage = 
                        reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE_VARIABLE_TYPE *>(pg->GetData());
                    result += interPage->ToString(verbose); 
                    result += "\n";
                    // add children
                    for(int i=0; i<interPage->GetSize(); ++i) {
                        page_id_t child_pid = interPage->ValueAt(i);
                        page_queue.push(child_pid);
                    }
                } else {
                    auto leafPage = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>(pg->GetData());
                    result += leafPage->ToString(verbose); 
                    result += "\n";
                }
                buffer_pool_manager_->UnpinPage(page_id, false);
                page_queue.pop();
            }
            
            return result; 
        }

    /*
     * This method is used for test only
     * Read data from file and insert one by one
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name,
                Transaction *transaction) {
            int64_t key;
            std::ifstream input(file_name);
            while (input) {
                input >> key;

                KeyType index_key;
                index_key.SetFromInteger(key);
                RID rid(key);
                Insert(index_key, rid, transaction);
            }
        }
    /*
     * This method is used for test only
     * Read data from file and remove one by one
     */
    INDEX_TEMPLATE_ARGUMENTS
        void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name,
                Transaction *transaction) {
            int64_t key;
            std::ifstream input(file_name);
            while (input) {
                input >> key;
                KeyType index_key;
                index_key.SetFromInteger(key);
                Remove(index_key, transaction);
            }
        }

    template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
    template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
    template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
    template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
    template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

} // namespace cmudb
