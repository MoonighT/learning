#pragma once

namespace featurestorage {
    template <typename K, typename V> class Store {
    public:
    Store() {}
    virtual ~Store() {}
    virtual void Insert(const K &key, const V &value) = 0;
    virtual void Get(const K &key, V *value) = 0;
    };
}
