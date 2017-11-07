/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <mutex>

#include "hash/hash_table.h"

namespace cmudb {


template <typename K, typename V>
class HashPage {
public:
    HashPage(size_t size);
    bool IsFull();
    bool Find(const K &key, V &value);
    bool Remove(const K &key);
    void Insert(const K &key, const V &value);
    int GetLocalDepth() const;
    void SetLocalDepth(const int depth);
    std::vector<std::tuple<K, V>> GetElements() const;
private:
    size_t _size;
    int _depth;
    std::vector<std::tuple<K, V>> _elements;
};

template <typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {
public:
  // constructor
  ExtendibleHash();
  ExtendibleHash(size_t size);
  // helper function to generate hash addressing
  size_t HashKey(const K &key);
  // helper function to get global & local depth
  int GetGlobalDepth() const;
  int GetLocalDepth(int bucket_id) const;
  int GetNumBuckets() const;
  // lookup and modifier
  bool Find(const K &key, V &value) override;
  bool Remove(const K &key) override;
  void Insert(const K &key, const V &value) override;

private:
  // add your own member variables here
  int _globalDepth;
  int _size;
  std::vector<std::shared_ptr<HashPage<K, V>>> _buckets;
  mutable std::mutex _hash_mutex;
};
} // namespace cmudb
