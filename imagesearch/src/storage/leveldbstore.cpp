#include <storage/leveldbstore.h>
#include <glog/logging.h>

namespace featurestorage {
    template <typename K, typename V>
    LeveldbStore<K, V>::LeveldbStore(std::string dbfile) {
        leveldb::Options options;
        options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(options, dbfile, &db); 
        if(!status.ok()) {
            LOG(WARNING) << "open db status error";
            return;
        } else {
            LOG(INFO) << "open db ok";
        }
    }

    template <typename K, typename V>
    void LeveldbStore<K, V>::Insert(const K &key, const V &value) {
        leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);           
        if(!s.ok()) {
            LOG(WARNING) << "put in db error";
        }
    }

    template <typename K, typename V>
    void LeveldbStore<K, V>::Get(const K &key, V *value) {
        leveldb::Status s = db->Get(leveldb::ReadOptions(), key, value);           
        if(!s.ok()) {
            LOG(WARNING) << "get from db error";
        }
    }

template class LeveldbStore<std::string, std::string>;

 }
