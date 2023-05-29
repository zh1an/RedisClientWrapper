//
// Created by Zh1an on 2023/5/29.
//

#include "redisclientwrapper.h"

#include <sw/redis++/redis++.h>

#include <thread>

namespace redis_client_wrapper {
    class redis_client {
    public:
        sw::redis::ConnectionOptions connectionOptions_{};
        sw::redis::ConnectionPoolOptions connectionPoolOptions_{};

        uint64_t wServiceKind_{};
        uint64_t wGameID_{};
        uint64_t wServerID_{};

        bool bCluster_mode_{};

        //private:
        std::shared_ptr<sw::redis::Redis> client_{};
        //sw::redis::RedisCluster clusterClient_;
    };

    enum class RCWErrorCode {
        success = 0,
        unknownError,
        IoError,
        TimeoutError,
        ClosedError,
        ProtoError,
        OomError,
        ReplyError,
        WatchError,
        NoInit,
        SystemError,
        KeyOrFieldNoExist,
    };

    class RCWCategory : public std::error_category {
    public:
        const char *name() const noexcept override { return "RCWCategory"; }

        std::string message(int ev) const override {
            switch (static_cast<RCWErrorCode>(ev)) {
                case RCWErrorCode::success:
                    return "Success";
                case RCWErrorCode::IoError:
                    return "IoError";
                case RCWErrorCode::TimeoutError:
                    return "TimeoutError";
                case RCWErrorCode::ClosedError:
                    return "ClosedError";
                case RCWErrorCode::ProtoError:
                    return "ProtoError";
                case RCWErrorCode::OomError:
                    return "OOMError";
                case RCWErrorCode::ReplyError:
                    return "ReplyError";
                case RCWErrorCode::WatchError:
                    return "WatchError";
                case RCWErrorCode::NoInit:
                    return "No init";
                case RCWErrorCode::SystemError:
                    return "SystemError";
                case RCWErrorCode::KeyOrFieldNoExist:
                    return "Key or Field is not exist";
                case RCWErrorCode::unknownError:
                default:
                    return "unknownError";
            }
        }

        static RCWCategory const &instance() {
            static RCWCategory w;
            return w;
        }
    };

    inline std::error_code makeRCWErrorCode(const RCWErrorCode &errorCode) {
        return {static_cast<int>(errorCode), RCWCategory::instance()};
    }

#define MAKE_ERROR_CODE(x) makeRCWErrorCode(x)
#define MAKE_SUCCESS_CODE MAKE_ERROR_CODE(RCWErrorCode::success)
#define MAKE_NO_EXIST MAKE_ERROR_CODE(RCWErrorCode::KeyOrFieldNoExist)

}// namespace redis_client_wrapper

using namespace redis_client_wrapper;

#define REDIS_CHECK                                                                                          \
    if (redisClient_->client_ == nullptr) { return MAKE_ERROR_CODE(RCWErrorCode::NoInit); }                  \
    try {

#define REDIS_END                                                                                            \
    }                                                                                                        \
    catch (const sw::redis::TimeoutError & /*IoError*/) {                                                    \
        if (redisClient_->client_ != nullptr) {                                                              \
            redisClient_->client_ = nullptr;                                                                 \
        }                                                                                                    \
        return MAKE_ERROR_CODE(RCWErrorCode::IoError);                                                       \
    }                                                                                                        \
    catch (const sw::redis::IoError & /*TimeoutError*/) {                                                    \
        if (redisClient_->client_ != nullptr) {                                                              \
            redisClient_->client_ = nullptr;                                                                 \
        }                                                                                                    \
        return MAKE_ERROR_CODE(RCWErrorCode::TimeoutError);                                                  \
    }                                                                                                        \
    catch (const sw::redis::ClosedError & /*ClosedError*/) {                                                 \
        if (redisClient_->client_ != nullptr) {                                                              \
            redisClient_->client_ = nullptr;                                                                 \
        }                                                                                                    \
        return MAKE_ERROR_CODE(RCWErrorCode::ClosedError);                                                   \
    }                                                                                                        \
    catch (const sw::redis::ProtoError & /*ProtoError*/) {                                                   \
        return MAKE_ERROR_CODE(RCWErrorCode::ProtoError);                                                    \
    }                                                                                                        \
    catch (const sw::redis::OomError & /*OomError*/) {                                                       \
        if (redisClient_->client_ != nullptr) {                                                              \
            redisClient_->client_ = nullptr;                                                                 \
        }                                                                                                    \
        return MAKE_ERROR_CODE(RCWErrorCode::OomError);                                                      \
    }                                                                                                        \
    catch (const sw::redis::ReplyError & /*ReplyError*/) {                                                   \
        return MAKE_ERROR_CODE(RCWErrorCode::ReplyError);                                                    \
    }                                                                                                        \
    catch (const sw::redis::WatchError & /*WatchError*/) {                                                   \
        return MAKE_ERROR_CODE(RCWErrorCode::WatchError);                                                    \
    }                                                                                                        \
    catch (const sw::redis::Error & /*error*/) {                                                             \
        return MAKE_ERROR_CODE(RCWErrorCode::unknownError);                                                  \
    }                                                                                                        \
    catch (...) {                                                                                            \
        return MAKE_ERROR_CODE(RCWErrorCode::SystemError);                                                   \
    }

RedisClientWrapper::RedisClientWrapper() : redisClient_(nullptr) {}

RedisClientWrapper::~RedisClientWrapper() {
    if (redisClient_) {
        delete redisClient_;
        redisClient_ = nullptr;
    }
}

std::error_code RedisClientWrapper::Init(uint64_t wServiceKind, uint64_t wGameID, uint64_t wServerID,
                                         std::string strRedisIP, int iRedisPort, std::string strRedisPasswd,
                                         bool bCluster_mode) {
    if (redisClient_->client_ != nullptr) { return MAKE_ERROR_CODE(RCWErrorCode::unknownError); }

    redisClient_->connectionOptions_.host = std::move(strRedisIP);
    redisClient_->connectionOptions_.port = iRedisPort;
    redisClient_->connectionOptions_.password = std::move(strRedisPasswd);

    redisClient_->connectionPoolOptions_.size = std::thread::hardware_concurrency();

    redisClient_->bCluster_mode_ = bCluster_mode;
    redisClient_->wGameID_ = wGameID;
    redisClient_->wServerID_ = wServerID;
    redisClient_->wServiceKind_ = wServiceKind;

    return MAKE_SUCCESS_CODE;
}

std::error_code RedisClientWrapper::StartService() {
    if (redisClient_->client_ == nullptr) {
        redisClient_->client_ = std::make_shared<sw::redis::Redis>(redisClient_->connectionOptions_,
                                                                   redisClient_->connectionPoolOptions_);
    }

    return MAKE_SUCCESS_CODE;
}

std::error_code RedisClientWrapper::ConcludeService() {
    if (redisClient_->client_ != nullptr) {
        redisClient_->client_ = nullptr;
    }

    return MAKE_SUCCESS_CODE;
}

std::error_code RedisClientWrapper::Redis_HmSet(const std::string &strKey,
                                                const std::map<std::string, std::string> &mapUpdateToRedis) {
    REDIS_CHECK

    redisClient_->client_->hmset(strKey, mapUpdateToRedis.begin(), mapUpdateToRedis.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_HmGet(const std::string &strKey,
                                                const std::vector<std::string> &vecFieldName,
                                                std::vector<std::string> &vecOutFieldVal) {
    REDIS_CHECK

    redisClient_->client_->hmget(strKey, vecFieldName.begin(), vecFieldName.end(),
                                 std::back_inserter(vecOutFieldVal));
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_HmDel(const std::string &strKey) {
    REDIS_CHECK

    std::unordered_map<std::string, std::string> results;
    redisClient_->client_->hgetall(strKey, std::inserter(results, results.begin()));

    std::vector<std::string> fields;
    fields.reserve(results.size());
    for (const auto &result: results) { fields.emplace_back(result.first); }

    redisClient_->client_->hdel(strKey, fields.begin(), fields.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Hexists(const std::string &strKey,
                                                  const std::string &strFieldName) {
    REDIS_CHECK

    auto result = redisClient_->client_->hexists(strKey, strFieldName);
    if (result) {
        return MAKE_SUCCESS_CODE;
    } else {
        return MAKE_NO_EXIST;
    }

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Hincrby(const std::string &strKey, const std::string &strFieldName,
                                                  int64_t iAdd, int64_t *result) {
    REDIS_CHECK

    *result = redisClient_->client_->hincrby(strKey, strFieldName, iAdd);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Hincrbyfloat(const std::string &strKey,
                                                       const std::string &strFieldName, double fAdd,
                                                       double *result) {
    REDIS_CHECK

    *result = redisClient_->client_->hincrbyfloat(strKey, strFieldName, fAdd);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_HmDelField(const std::string &strKey,
                                                     const std::string &strFieldName) {
    REDIS_CHECK

    redisClient_->client_->hdel(strKey, strFieldName);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Hkeys(const std::string &strKey,
                                                std::vector<std::string> &vecOutFieldName) {
    REDIS_CHECK

    redisClient_->client_->hkeys(strKey, std::back_inserter(vecOutFieldName));
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_StringSetValue(const std::string &strKey,
                                                         const std::string &strSetVal) {
    REDIS_CHECK

    redisClient_->client_->set(strKey, strSetVal);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_StringGetValue(const std::string &strKey, std::string &strOutVal) {
    REDIS_CHECK

    auto opt = redisClient_->client_->get(strKey);
    if (opt) {
        strOutVal = *opt;
        return MAKE_SUCCESS_CODE;
    }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_StringIncrby(const std::string &strKey, int64_t iAdd,
                                                       int64_t *result) {
    REDIS_CHECK

    *result = redisClient_->client_->incrby(strKey, iAdd);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_keyExists(const std::string &strKey) {
    REDIS_CHECK

    auto result = redisClient_->client_->exists(strKey);
    if (result) { return MAKE_SUCCESS_CODE; }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_KeyDel(const std::string &strKey) {
    REDIS_CHECK

    redisClient_->client_->del(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_KeyExpire(const std::string &strKey, int iAliveSec) {
    REDIS_CHECK

    auto result = redisClient_->client_->expire(strKey, iAliveSec);
    if (result) { return MAKE_SUCCESS_CODE; }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_KeyPersist(const std::string &strKey) {
    REDIS_CHECK

    auto result = redisClient_->client_->persist(strKey);
    if (result) { return MAKE_SUCCESS_CODE; }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_KeyTtl(const std::string &strKey, uint64_t &ttl) {
    REDIS_CHECK

    ttl = redisClient_->client_->ttl(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_KeyType(const std::string &strKey, int &type) {
    REDIS_CHECK

    auto sType = redisClient_->client_->type(strKey);

    // redis 服务支持的数据类型分类
    // the data type supported by redis
    typedef enum {
        REDIS_KEY_NONE,  // none
        REDIS_KEY_STRING,// string
        REDIS_KEY_HASH,  // hash
        REDIS_KEY_LIST,  // list
        REDIS_KEY_SET,   // set
        REDIS_KEY_ZSET   // sorted set
    } redis_key_t;

    static const std::map<std::string, int> typeWithValue = {
            {"string", redis_key_t::REDIS_KEY_STRING},
            {"list", redis_key_t::REDIS_KEY_LIST},
            {"set", redis_key_t::REDIS_KEY_SET},
            {"zset", redis_key_t::REDIS_KEY_ZSET},
            {"hash", redis_key_t::REDIS_KEY_HASH} /*,
                {"stream", redis_key_t ::REDIS_KEY_HASH}*/
    };
    if (typeWithValue.find(sType) == typeWithValue.end()) {
        type = typeWithValue.at(sType);
        return MAKE_SUCCESS_CODE;
    }

    type = redis_key_t::REDIS_KEY_NONE;
    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetAddOne(const std::string &strKey, const std::string &strMember) {
    REDIS_CHECK

    redisClient_->client_->sadd(strKey, strMember);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetAddLs(const std::string &strKey,
                                                   const std::vector<const char *> &vecMember) {
    REDIS_CHECK

    redisClient_->client_->sadd(strKey, vecMember.begin(), vecMember.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetDelOne(const std::string &strKey, const std::string &strMember) {
    REDIS_CHECK

    redisClient_->client_->srem(strKey, strMember);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetDelLs(const std::string &strKey,
                                                   const std::vector<std::string> &vecMember) {
    REDIS_CHECK

    redisClient_->client_->srem(strKey, vecMember.begin(), vecMember.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetRemoveAll(const std::string &strKey) {
    REDIS_CHECK

    redisClient_->client_->del(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetReturnAll(const std::string &strKey,
                                                       std::vector<std::string> &vecOutMembers) {
    REDIS_CHECK

    redisClient_->client_->smembers(strKey, std::back_inserter(vecOutMembers));
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetIsMember(const std::string &strKey,
                                                      const std::string &strMember) {
    REDIS_CHECK

    auto result = redisClient_->client_->sismember(strKey, strMember);
    if (result) { return MAKE_SUCCESS_CODE; }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_SetMemCount(const std::string &strKey, uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->scard(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zadd(const std::string &strKey,
                                                    const std::map<std::string, double> &members,
                                                    uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->zadd(strKey, members.begin(), members.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zcard(const std::string &strKey, uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->zcard(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zincrby(const std::string &strKey, double inc,
                                                       const std::string &strMember, double *result) {
    REDIS_CHECK

    *result = redisClient_->client_->zincrby(strKey, inc, strMember);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zrange(const std::string &strKey, bool bAsc, int start,
                                                      int stop, std::vector<std::string> *result,
                                                      uint64_t &count) {
    REDIS_CHECK
    if (bAsc) {
        redisClient_->client_->zrange(strKey, start, stop, std::back_inserter(*result));
    } else {
        redisClient_->client_->zrevrange(strKey, start, stop, std::back_inserter(*result));
    }

    return MAKE_SUCCESS_CODE;
    REDIS_END
}

std::error_code
RedisClientWrapper::Redis_Zset_zrange_with_scores(const std::string &strKey, bool bAsc, int start, int stop,
                                                  std::vector<std::pair<std::string, double>> &vecOut) {
    REDIS_CHECK
    if (bAsc) {
        redisClient_->client_->zrange(strKey, start, stop, std::back_inserter(vecOut));
    } else {
        redisClient_->client_->zrevrange(strKey, start, stop, std::back_inserter(vecOut));
    }

    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zrangebyscore_with_scores(
        const std::string &strKey, bool bAsc, double min, double max,
        std::vector<std::pair<std::string, double>> &vecOut, const int *offset, const int *count,
        uint64_t &result) {
    REDIS_CHECK

    sw::redis::BoundedInterval<double> boundedInterval(min, max, sw::redis::BoundType::CLOSED);
    sw::redis::LimitOptions limitOptions;
    limitOptions.count = *count;
    limitOptions.offset = *offset;

    if (bAsc) {
        redisClient_->client_->zrangebyscore(strKey, boundedInterval, limitOptions,
                                             std::back_inserter(vecOut));
    } else {
        redisClient_->client_->zrevrangebyscore(strKey, boundedInterval, limitOptions,
                                                std::back_inserter(vecOut));
    }
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zrem(const std::string &strKey,
                                                    const std::vector<std::string> &members,
                                                    uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->zrem(strKey, members.begin(), members.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zremrangebyrank(const std::string &strKey, int start, int stop,
                                                               uint64_t &count) {
    REDIS_CHECK
    count = redisClient_->client_->zremrangebyrank(strKey, start, stop);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zremrangebyscore(const std::string &strKey, double min,
                                                                double max, uint64_t &count) {
    REDIS_CHECK
    sw::redis::BoundedInterval<double> boundedInterval(min, max, sw::redis::BoundType::CLOSED);

    count = redisClient_->client_->zremrangebyscore(strKey, boundedInterval);
    return MAKE_SUCCESS_CODE;
    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zunionstore(const std::string &strDstKey,
                                                           const std::vector<std::string> &keys,
                                                           const std::vector<double> *weights,
                                                           uint64_t &count, const char *aggregate) {
    REDIS_CHECK

    auto agg = std::string(aggregate);
    auto type = sw::redis::Aggregation::MAX;
    if (agg == "MIN") {
        type = sw::redis::Aggregation::MIN;
    } else if (agg == "SUM") {
        type = sw::redis::Aggregation::SUM;
    }

    if (keys.size() != weights->size()) { return MAKE_ERROR_CODE(RCWErrorCode::SystemError); }

    std::vector<std::pair<std::string, double>> keyWithWeights;
    keyWithWeights.reserve(keys.size());
    for (auto i = 0U; i < keys.size(); ++i) { keyWithWeights.emplace_back(keys.at(i), weights->at(i)); }

    count = redisClient_->client_->zunionstore(strDstKey, keyWithWeights.begin(), keyWithWeights.end(), type);

    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_zscore(const std::string &strKey, const std::string &strMember,
                                                      double &result) {
    REDIS_CHECK

    auto option = redisClient_->client_->zscore(strKey, strMember);
    if (option) {
        result = *option;
        return MAKE_SUCCESS_CODE;
    }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Zset_IsExists(const std::string &strKey,
                                                        const std::string &strMember) {
    REDIS_CHECK

    auto opt = redisClient_->client_->zrank(strKey, strMember);
    if (!opt) { return MAKE_SUCCESS_CODE; }
    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_blpop(const std::vector<std::string> &keys, size_t timeout,
                                                     std::pair<std::string, std::string> &result) {
    REDIS_CHECK

    auto option = redisClient_->client_->blpop(keys.begin(), keys.end(), static_cast<long long>(timeout));
    if (!option) { return MAKE_NO_EXIST; }

    result = *option;
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_brpop(const std::vector<std::string> &keys, size_t timeout,
                                                     std::pair<std::string, std::string> &result) {
    REDIS_CHECK

    auto option = redisClient_->client_->brpop(keys.begin(), keys.end(), static_cast<long long>(timeout));
    if (!option) { return MAKE_NO_EXIST; }

    result = *option;
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_llen(const std::string &strKey, uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->llen(strKey);
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_lpop(const std::string &strKey, std::string &strOutData,
                                                    uint64_t &count) {
    REDIS_CHECK

    auto option = redisClient_->client_->lpop(strKey);
    if (option) {
        strOutData = *option;
        return MAKE_SUCCESS_CODE;
    }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_rpop(const std::string &strKey, std::string &strOutData) {
    REDIS_CHECK

    auto option = redisClient_->client_->rpop(strKey);
    if (option) {
        strOutData = *option;
        return MAKE_SUCCESS_CODE;
    }

    return MAKE_NO_EXIST;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_lpush(const std::string &strKey,
                                                     const std::vector<std::string> &values,
                                                     uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->lpush(strKey, values.begin(), values.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_List_rpush(const std::string &strKey,
                                                     const std::vector<std::string> &values,
                                                     uint64_t &count) {
    REDIS_CHECK

    count = redisClient_->client_->rpush(strKey, values.begin(), values.end());
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::Redis_Server_Bgsave() {
    REDIS_CHECK

    redisClient_->client_->bgsave();
    return MAKE_SUCCESS_CODE;

    REDIS_END
}

std::error_code RedisClientWrapper::IsConnect(bool auto_connect) {
    if (redisClient_->client_) { return MAKE_SUCCESS_CODE; }

    if (auto_connect) { return StartService(); }

    return MAKE_ERROR_CODE(RCWErrorCode::NoInit);
}
