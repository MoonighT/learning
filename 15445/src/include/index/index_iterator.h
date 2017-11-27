/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "page/b_plus_tree_leaf_page.h"

namespace cmudb {

#define INDEXITERATOR_TYPE                                                     \
  IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
public:
  // you may define your own constructor based on your member variables
  IndexIterator();
  IndexIterator(page_id_t, page_id_t, int, int, BufferPoolManager*);
  ~IndexIterator();

  bool isEnd();

  const MappingType &operator*();

  IndexIterator &operator++();

  inline std::string ToString() const {
    std::stringstream os;
    os << "page_id: " << page_id_;
    os << " size: " << size_ ;
    os << " pos: " << pos_ << "\n";
    return os.str();
  }

private:
  // add your own private member variables here
  page_id_t page_id_;
  page_id_t next_pg_id_;
  int pos_;
  int size_;
  BufferPoolManager *buffer_pool_manager_;
};

} // namespace cmudb
