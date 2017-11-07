#include <list>
#include <functional>
#include <memory>

#include "hash/extendible_hash.h"
#include "page/page.h"

namespace cmudb {


template <typename K, typename V>
HashPage<K, V>::HashPage(size_t size): _size(size), _depth(0){
    //_elements = std::vector<std::tuple<K,V>>();
}

template <typename K, typename V>
bool HashPage<K, V>::IsFull() {
    if (_elements.size() == _size) {
        return true;
    }
    return false;
}

template <typename K, typename V>
bool HashPage<K, V>::Find(const K &key, V &value) {
    for (auto t : _elements) {
        if(std::get<0>(t) == key) {
            value = std::get<1>(t);
            return true;
        }
    }
    return false;
}

template <typename K, typename V>
bool HashPage<K, V>::Remove(const K &key) {
    for (auto it = _elements.begin(); it != _elements.end(); ++it) {
        if(std::get<0>(*it) == key) {
            //remove from vector
            _elements.erase(it);
            return true;
        }
    }
    return false;
}

template <typename K, typename V>
void HashPage<K, V>::Insert(const K &key, const V &value) {
    _elements.push_back(std::make_tuple(key, value));
}

template <typename K, typename V>
int HashPage<K, V>::GetLocalDepth() const {
    return  _depth;
}

template <typename K, typename V>
void HashPage<K, V>::SetLocalDepth(const int depth) {
    _depth = depth;
}

template <typename K, typename V>
std::vector<std::tuple<K, V>> HashPage<K,V>::GetElements() const {
    return _elements;
}
/*
 * constructor
 * array_size: fixed array size for each bucket
 */

template <typename k, typename v>
ExtendibleHash<k, v>::ExtendibleHash() {
    //default size
    _size = 8;
    _globalDepth = 0;
    _buckets.resize(1);
    _buckets[0] = std::make_shared<HashPage<k, v>>(_size); 
}

template <typename k, typename v>
ExtendibleHash<k, v>::ExtendibleHash(size_t size) {
    _globalDepth = 0;
    _size = size;
    _buckets.resize(1);
    _buckets[0] = std::make_shared<HashPage<k, v>>(size); 
}

/*
 * helper function to calculate the hashing address of input key
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key) {
  return std::hash<K>{}(key);
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const {
  std::lock_guard<std::mutex> lg(_hash_mutex);
  return _globalDepth;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
  std::lock_guard<std::mutex> guard(_hash_mutex);
  return _buckets[bucket_id]->GetLocalDepth();
}

/*
 * helper function to return current number of bucket in hash table
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const {
  std::lock_guard<std::mutex> guard(_hash_mutex);
  return _buckets.size();
}

/*
 * lookup function to find value associate with input key
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
  std::lock_guard<std::mutex> guard(_hash_mutex);
  size_t hashval = HashKey(key);
  int depth = _globalDepth;
    int bucketId = hashval & ((1 << depth) -1);
    //printf("Find|bucketid=%d,globaldepth=%d,localdepth=%d\n", bucketId, depth, 
    //        _buckets[bucketId]->GetLocalDepth());
  return _buckets[bucketId]->Find(key, value);
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key) {
  std::lock_guard<std::mutex> guard(_hash_mutex);
  size_t hashval = HashKey(key);
  int depth = _globalDepth;
  int bucketId = hashval & ((1 << depth) -1);
    //printf("Remove|bucketid=%d,globaldepth=%d,localdepth=%d\n", bucketId, depth, 
    //        _buckets[bucketId]->GetLocalDepth());
  return _buckets[bucketId]->Remove(key);
}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template <typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
    std::lock_guard<std::mutex> guard(_hash_mutex);
    
    size_t hashval = HashKey(key);
    //printf("Insert|hashval=%ld,globaldepth=%d\n",
    //            hashval, _globalDepth);

    while(true) {
        size_t bucketId = hashval & ((1 << _globalDepth) -1);
        //printf("Insert|bucketId=%ld,hashval=%ld,globaldepth=%d,size=%ld\n",
        //        bucketId, hashval, _globalDepth, _buckets[bucketId]->GetElements().size());
        //for(size_t i=0; i<_buckets.size(); ++i) {
        //    printf("bucket i = %ld\n", i);
        //    for(auto ele: _buckets[i]->GetElements()) {
        //        printf("element=%ld\n", HashKey(std::get<0>(ele)));
        //    }
        //}
        if(_buckets[bucketId]->IsFull() && 
                _buckets[bucketId]->GetLocalDepth() == _globalDepth) {
            //printf("Insert|bucket=%ld,full=true,localdepth=globaldepth=%d\n",bucketId, _globalDepth);
            _globalDepth++;
            _buckets.resize(2*_buckets.size());
            for(size_t i= _buckets.size()/2; i < _buckets.size(); ++i) {
                _buckets[i] = _buckets[i-_buckets.size()/2];
            }
        }
        if(_buckets[bucketId]->IsFull() && 
                _buckets[bucketId]->GetLocalDepth() < _globalDepth) {
            //printf("Insert|bucket=%ld,full=true,localdepth=%d<globaldepth=%d\n",
             //       bucketId, _buckets[bucketId]->GetLocalDepth(), _globalDepth);
            auto elements = _buckets[bucketId]->GetElements();
            auto temp = std::make_shared<HashPage<K, V>>(_size);
            bucketId = bucketId & ((1 << _buckets[bucketId]->GetLocalDepth()) -1);
            size_t another = (1 << (_globalDepth-1)) | bucketId;
            _buckets[another] = std::make_shared<HashPage<K, V>>(_size);
            for(auto ele : elements) {
                size_t newId = HashKey(std::get<0>(ele)) & ((1 << _globalDepth) -1);
                //printf("Insert|newId=%ld,globaldepth=%d\n", newId, _globalDepth);
                if( newId == bucketId) {
                    temp->Insert(std::get<0>(ele), std::get<1>(ele));
                } else {
                    _buckets[newId]->Insert(std::get<0>(ele), std::get<1>(ele));
                }
            }
            temp->SetLocalDepth(_buckets[bucketId]->GetLocalDepth()+1);
            _buckets[another]->SetLocalDepth(_buckets[bucketId]->GetLocalDepth()+1);
            _buckets[bucketId] = temp;
            //printf("Insert_setlocaldepth,bucket=%ld,depth=%d,another=%ld,depth=%d\n", 
             //       bucketId, _buckets[bucketId]->GetLocalDepth(), another,_buckets[another]->GetLocalDepth());
            continue;
            // split
        } else {
            //printf("Insert_ok|bucketId=%ld\n", bucketId);
            _buckets[bucketId]->Insert(key, value);
            break;
        }
    }
}

template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// hash page
template class HashPage<page_id_t, Page *>;
template class HashPage<Page *, std::list<Page *>::iterator>;
// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;
// hash page
template class HashPage<int, std::string>;
template class HashPage<int, std::list<int>::iterator>;
template class HashPage<int, int>;
} // namespace cmudb
