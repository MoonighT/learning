#pragma once
#include <storage/store.h>
#include <string>
#include <leveldb/db.h>

namespace featurestorage {

    template <typename K, typename V> 
    class LeveldbStore : public Store<K,V> {
        public:
        LeveldbStore(std::string dbfile);
        ~LeveldbStore() {};

        void Insert(const K &key, const V &value) override;
        void Get(const K &key, V *value) override;

        private:
        leveldb::DB* db;
    };
}
