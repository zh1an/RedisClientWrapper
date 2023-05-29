//
// Created by Zh1an on 2023/5/29.
//
#include <gtest/gtest.h>

#include <redisclientwrapper.h>

RedisClientWrapper *client = nullptr;

TEST(RedisClientWrapper, Init) {
    client = new RedisClientWrapper;

    auto result = client->Init(1, 1, 1, "127.0.0.1", 6379, "password");
    EXPECT_EQ(result.value(), 0);
}

TEST(RedisClientWrapper, StartService) {
    auto result = client->StartService();
    EXPECT_EQ(result.value(), 0);
}

TEST(RedisClientWrapper, HMSet) {
    std::string strKey = "Game:GB_TestNormal:10001";
    std::map<std::string, std::string> updateToRedis;
    updateToRedis["LastLogonDate"] = "2023-04-27 09:06:55";
    updateToRedis["LastLogonIP"] = "127.0.0.1";
    updateToRedis["LastLogonMachine"] = "aaaaa";
    updateToRedis["UserID"] = "10001";

    auto result = client->Redis_HmSet(strKey, updateToRedis);
    EXPECT_EQ(result.value(), 0);
}

TEST(RedisClientWrapper, HMGet) {
    std::string strKey = "Game:GB_TestNormal:10001";

    std::vector<std::string> vecOutFieldVal;
    std::vector<std::string> fieldKey = {"LastLogonDate", "LastLogonIP", "LastLogonMachine", "UserID"};
    std::vector<std::string> fieldValue = {"2023-04-27 09:06:55", "127.0.0.1", "aaaaa", "10001"};
    auto result = client->Redis_HmGet(strKey, fieldKey, vecOutFieldVal);

    EXPECT_TRUE(vecOutFieldVal == fieldValue);
    EXPECT_EQ(result.value(), 0);
}

TEST(RedisClientWrapper, HMExist) {
    std::string strKey = "Game:GB_TestNormal:10001";
    std::string fieldKey = "LastLogonDate";

    auto result = client->Redis_Hexists(strKey, fieldKey);

    EXPECT_EQ(result.value(), 0);
}

TEST(RedisClientWrapper, HMDelField) {
    std::string strKey = "Game:GB_TestNormal:10001";
    std::string fieldKey = "LastLogonDate";

    auto result = client->Redis_HmDelField(strKey, fieldKey);
    EXPECT_EQ(result.value(), 0);

    result = client->Redis_Hexists(strKey, fieldKey);
    EXPECT_NE(result.value(), 0);
}

TEST(RedisClientWrapper, HMDel) {
    std::string strKey = "Game:GB_TestNormal:10001";

    auto result = client->Redis_HmDel(strKey);

    EXPECT_EQ(result.value(), 0);

    std::string fieldKey = "LastLogonDate";
    result = client->Redis_Hexists(strKey, fieldKey);
    EXPECT_NE(result.value(), 0);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}