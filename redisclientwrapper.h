//
// Created by 兰钊 on 2023/5/27.
//

#ifndef REDISCLIENTWRAPPER_REDISCLIENTWRAPPER_H
#define REDISCLIENTWRAPPER_REDISCLIENTWRAPPER_H

#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#if defined(_MSC_VER)
#if defined(REDIS_CLIENT_WRAPPER_SHARED)
#define REDIS_CLIENT_WRAPPER_EXPORT __declspec(dllexport)
#else
#define REDIS_CLIENT_WRAPPER_EXPORT __declspec(dllimport)
#endif
#else
#define
#endif

namespace redis_client_wrapper {
    class redis_client;
}


class REDIS_CLIENT_WRAPPER_EXPORT RedisClientWrapper {
public:
    RedisClientWrapper();
    ~RedisClientWrapper();

public:
    //初始化
    std::error_code Init(uint64_t wServiceKind, uint64_t wGameID, uint64_t wServerID, std::string strRedisIP,
                         int iRedisPort, std::string strRedisPasswd, bool bCluster_mode = false);

    //启动服务
    std::error_code StartService();

    //停止服务
    std::error_code ConcludeService();

    /***redis hash*************************************************************************/
    //设置hash 中某些字段的值
    std::error_code Redis_HmSet(const std::string &strKey,
                                const std::map<std::string, std::string> &mapUpdateToRedis);

    //获取hash 中某些字段的值
    std::error_code Redis_HmGet(const std::string &strKey, const std::vector<std::string> &vecFieldName,
                                std::vector<std::string> &vecOutFieldVal);

    //删除hash
    std::error_code Redis_HmDel(const std::string &strKey);

    //判断hash中是否存在某Field
    std::error_code Redis_Hexists(const std::string &strKey, const std::string &strFieldName);

    //增加hash key中某个字段的值，注意字段必须是数字的
    std::error_code Redis_Hincrby(const std::string &strKey, const std::string &strFieldName, int64_t iAdd,
                                  int64_t *result);

    std::error_code Redis_Hincrbyfloat(const std::string &strKey, const std::string &strFieldName,
                                       double fAdd, double *result);

    //删除hash 的Field
    std::error_code Redis_HmDelField(const std::string &strKey, const std::string &strFieldName);

    //获取hash key对应的所有Field名列表
    std::error_code Redis_Hkeys(const std::string &strKey, std::vector<std::string> &vecOutFieldName);


    /***redis String*************************************************************************/
    //redis String获取设置
    std::error_code Redis_StringSetValue(const std::string &strKey, const std::string &strSetVal);

    std::error_code Redis_StringGetValue(const std::string &strKey, std::string &strOutVal);

    //将 key 所储存的值加上增量 iAdd
    std::error_code Redis_StringIncrby(const std::string &strKey, int64_t iAdd, int64_t *result);

    /***redis key操作*************************************************************************/
    //redis key是否存在
    std::error_code Redis_keyExists(const std::string &strKey);

    //redis key删除
    std::error_code Redis_KeyDel(const std::string &strKey);

    //设置key生存时间 参数iAliveSec 秒
    //返回值： > 0: 成功设置了生存周期, 0：该 key 不存在 , < 0: 出错
    std::error_code Redis_KeyExpire(const std::string &strKey, int iAliveSec);

    //移除给定 key 的生存时间，将这个 key 从"易失的"(带生存时间 key )转换成 "持久的"(一个不带生存时间、永不过期的 key )
    std::error_code Redis_KeyPersist(const std::string &strKey);

    //获得 KEY 的剩余生存周期，单位（秒）
    //  > 0: 该 key 剩余的生存周期（秒）
    //   -3：出错
    //   -2：key 不存在
    //   -1：当 key 存在但没有设置剩余时间
    std::error_code Redis_KeyTtl(const std::string &strKey, uint64_t &ttl);

    //获得 KEY 的存储类型 ,返回类型参考 acl::redis_key_t
    std::error_code Redis_KeyType(const std::string &strKey, int &type);


    /***redis set操作*************************************************************************/
    std::error_code Redis_SetAddOne(const std::string &strKey, const std::string &strMember);

    std::error_code Redis_SetAddLs(const std::string &strKey, const std::vector<const char *> &vecMember);

    std::error_code Redis_SetDelOne(const std::string &strKey, const std::string &strMember);

    std::error_code Redis_SetDelLs(const std::string &strKey, const std::vector<std::string> &vecMember);

    std::error_code Redis_SetRemoveAll(const std::string &strKey);

    std::error_code Redis_SetReturnAll(const std::string &strKey, std::vector<std::string> &vecOutMembers);

    std::error_code Redis_SetIsMember(const std::string &strKey, const std::string &strMember);

    std::error_code Redis_SetMemCount(const std::string &strKey, uint64_t &count);
    //int  Redis_SetScan(const std::string& strKey,int iCursor, std::vector<std::string>& vecOut,const size_t* iCount,const char* pPattern);

    /***redis zset操作*************************************************************************/
    //有序集合 增加
    //返回成功个数
    std::error_code Redis_Zset_zadd(const std::string &strKey, const std::map<std::string, double> &members,
                                    uint64_t &count);

    //获得相应键的有序集的成员数量
    std::error_code Redis_Zset_zcard(const std::string &strKey, uint64_t &count);

    //将 key 的有序集中的某个成员的分值加上增量 inc
    std::error_code Redis_Zset_zincrby(const std::string &strKey, double inc, const std::string &strMember,
                                       double *result);

    //功能:从 key 的有序集中获得指定位置区间的成员名列表，成员按分值bAsc方式排序
    //参数:bAsc  true 递增方式,false 递减方式
    //     start 起始下标位置
    //     stop  结束下标位置（结果集同时含该位置）
    //     对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员， -2 表示倒数第二个成员，以此类推
    //返回:结果集的数量 -1: 表示出错或 key 对象非有序集对象
    std::error_code Redis_Zset_zrange(const std::string &strKey, bool bAsc, int start, int stop,
                                      std::vector<std::string> *result, uint64_t &count);

    //功能:从 key 的有序集中获得指定位置区间的成员名及分值列表，成员按分值bAsc方式排序
    //参数:bAsc  true 递增方式,false 递减方式
    //     start 起始下标位置
    //     stop  结束下标位置（结果集同时含该位置）
    //     对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员， -2 表示倒数第二个成员，以此类推
    //返回:结果集的数量 -1: 表示出错或 key 对象非有序集对象
    std::error_code Redis_Zset_zrange_with_scores(const std::string &strKey, bool bAsc, int start, int stop,
                                                  std::vector<std::pair<std::string, double>> &vecOut);


    /**
    * 功能: 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
    *       的成员及分值。有序集成员按 score 值 依照 参数bAsc 次序排列；分值(min/max)使用浮点数表示
    * 参数: @param bAsc  true 递增方式,false 递减方式
    *       @param out 存储结果集，内部先调用 out.clear()
    * 返回: @return {int} 结果集中成员的数量
    */
    std::error_code Redis_Zset_zrangebyscore_with_scores(const std::string &strKey, bool bAsc, double min,
                                                         double max,
                                                         std::vector<std::pair<std::string, double>> &vecOut,
                                                         const int *offset, const int *count,
                                                         uint64_t &result);

    /**
    * 功能: 从有序集中删除某个成员
    *       的成员及分值。有序集成员按 score 值递增(从小到大)次序排列；分值(min/max)使用浮点数表示
    * 参数: @param key {const char*} 有序集键值
    * 返回: 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
    *       0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
    */
    std::error_code Redis_Zset_zrem(const std::string &strKey, const std::vector<std::string> &members,
                                    uint64_t &count);

    /**
     * 移除有序集 key 中，指定排名(rank)区间内的所有成员；
     * 区间分别以下标参数 start 和 stop 指出，包含 start 和 stop 在内；
     * 下标参数 start 和 stop 都以 0 为底，也就是说，以 0 表示有序集第一个成员，
     * 以 1 表示有序集第二个成员，以此类推；
     * 也可以使用负数下标，以 -1 表示最后一个成员， -2 表示倒数第二个成员，以此类推
     * @param key {const char*} 有序集键值
     * @param start {int} 起始下标位置（从 0 开始）
     * @param stop {int} 结束下标位置
     * @return {int} 被移除的成员数量
     *  0：表示 key 不存在或移除的区间不存在
     * -1：表示出错或 key 不是有序集合对象键值
     */
    std::error_code Redis_Zset_zremrangebyrank(const std::string &strKey, int start, int stop,
                                               uint64_t &count);

    /**
     * 移除有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
     * 的成员；自版本2.1.6开始，score 值等于 min 或 max 的成员也可以不包括在内，
     * 详情请参见 ZRANGEBYSCORE 命令
     * @param key {const char*} 有序集键值
     * @param min {double} 最小分值
     * @param max {double} 最大分值
     * @return {int} 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
     *  0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
     */
    std::error_code Redis_Zset_zremrangebyscore(const std::string &strKey, double min, double max,
                                                uint64_t &count);

    /**
    * 功能: 计算给定的一个或多个有序集的并集，其中给定 key 的数量必须以 numkeys 参数指定，
    *       并将该并集(结果集)储存到目标有序集; 默认情况下，结果集中某个成员的 score 是
    *       所有集合中某个成员的 最大 score 值
    * 参数: @param strDstKey {const char*} 目标有序集键值
    @param keys 源有序集键值-权重集合
    @param aggregate {const char*} 聚合方式，默认是 MAX 聚合方式，聚合方式如下：
    *       SUM: 将所有集合中某个成员的 score 值之 和 作为结果集中该成员的 score 值
    *       MIN: 将所有集合中某个成员的 最小 score 值作为结果集中该成员的 score 值
    *       MAX: 将所有集合中某个成员的 最大 score 值作为结果集中该成员的 score 值
    * 返回: 新保存到目标有序集的结果集中的元素(成员)数量，如果源有序集
    *       集合中存在相同的成员，则只新增一个成员；返回 -1 表示出错
    */
    std::error_code Redis_Zset_zunionstore(const std::string &strDstKey, const std::vector<std::string> &keys,
                                           const std::vector<double> *weights, uint64_t &count,
                                           const char *aggregate = "MAX");

    //功能:获取某个成员的分数
    //返回:当不存在或出错时返回 false，否则返回 true
    std::error_code Redis_Zset_zscore(const std::string &strKey, const std::string &strMember,
                                      double &result);

    //功能:判断是否存在
    //返回:当不存在或出错时返回 false，否则返回 true
    std::error_code Redis_Zset_IsExists(const std::string &strKey, const std::string &strMember);


    /***redis list操作*************************************************************************/
    /**
    * 功能: 从 key 列表对象中弹出一个元素对象（name/value对），采用阻塞方式从头部弹出；
    *		 当给定多个 key 参数时，按参数 key 的先后顺序依次检查各个列表，弹出第一个
    *		 非空列表的头元素
    * 参数: @param timeout 等待阻塞时间（秒），在超时时间内没有获得元素对象， 则返回 false；如果该值为 0 则一直等待至获得元素对象或出错
    *		@param result {std::pair<string, string>&} 存储结果元素对象，该对象的第一个字符串表示是列表对象的 key，第二个为该对象的头部元素
    */
    std::error_code Redis_List_blpop(const std::vector<std::string> &keys, size_t timeout,
                                     std::pair<std::string, std::string> &result);

    //含义参见 Redis_List_blpop，唯一区别为该方法弹出尾部元素对象
    std::error_code Redis_List_brpop(const std::vector<std::string> &keys, size_t timeout,
                                     std::pair<std::string, std::string> &result);

    //返回指定列表对象的长度（即元素个数）， -1 if error happened
    std::error_code Redis_List_llen(const std::string &strKey, uint64_t &count);

    /**
    * 功能: 从列表对象中移除并返回头部元素
    * 参数: strOutData 存储弹出的元素值
    * 返回: >0 -- 表示成功弹出一个元素且返回值表示元素的长度，
    *		-1 -- 表示出错，或该对象非列表对象，或该对象已经为空
    */
    std::error_code Redis_List_lpop(const std::string &strKey, std::string &strOutData, uint64_t &count);

    //从列表对象中移除并返回尾部元素
    std::error_code Redis_List_rpop(const std::string &strKey, std::string &strOutData);

    //功能:将一个或多个值元素插入到列表对象 key 的表头
    //返回:返回添加完后当前列表对象中的元素个数，返回 -1 表示出错或该 key 对象非列表对象，当该 key 不存在时会添加新的列表对象及对象中的元素
    std::error_code Redis_List_lpush(const std::string &strKey, const std::vector<std::string> &values,
                                     uint64_t &count);

    std::error_code Redis_List_rpush(const std::string &strKey, const std::vector<std::string> &values,
                                     uint64_t &count);


    //功能:在后台异步(Asynchronously)保存当前数据库的数据到磁盘 立即返回 OK
    //返回:发送命令成功 返回true，否则 false
    std::error_code Redis_Server_Bgsave();

    //功能:检测是否已连接
    //参数:auto_connect 表示如果没有连接，是否要进行连接操作
    //返回:true 表示已连接，false表示未连接
    std::error_code IsConnect(bool auto_connect = false);

private:
    redis_client_wrapper::redis_client *redisClient_;
};


#endif//REDISCLIENTWRAPPER_REDISCLIENTWRAPPER_H
