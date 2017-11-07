/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace cmudb {

    template <typename T> LRUReplacer<T>::LRUReplacer() {}

    template <typename T> LRUReplacer<T>::~LRUReplacer() {}

    /*
     * Insert value into LRU
     */
    template <typename T> void LRUReplacer<T>::Insert(const T &value) {
        // insert into end of list
        // add into hash map
        // if exist, just move it to back
        Erase(value);
        std::lock_guard<std::mutex> guard(_lru_mutex);
        _orderList.push_back(value);
        auto iter = _orderList.end();
        iter--;
        _map.Insert(value, iter);
    }

    /* If LRU is non-empty, pop the head member from LRU to argument "value", and
     * return true. If LRU is empty, return false
     */
    template <typename T> bool LRUReplacer<T>::Victim(T &value) {
        std::lock_guard<std::mutex> guard(_lru_mutex);
        if(Size() == 0)
            return false;
        value = _orderList.front();
        _map.Remove(_orderList.front());
        _orderList.pop_front();
        return true;
    }

    /*
     * Remove value from LRU. If removal is successful, return true, otherwise
     * return false
     */
    template <typename T> bool LRUReplacer<T>::Erase(const T &value) {
        std::lock_guard<std::mutex> guard(_lru_mutex);
        typename std::list<T>::iterator iter;
        if (_map.Find(value, iter)) {
            // has key
            _map.Remove(value);
            _orderList.erase(iter);
            return true;
        }
        return false;
    }

    template <typename T> size_t LRUReplacer<T>::Size() { 
        return _orderList.size(); 
    }

    template class LRUReplacer<Page *>;
    // test only
    template class LRUReplacer<int>;

} // namespace cmudb
