#include <boost/test/unit_test.hpp>

#include "LogItem.h"
#include "DjsonLogItem.h"
#include "EtwLogItem.h"
#include "IdMgr.h"

using namespace EndpointLog;

BOOST_AUTO_TEST_SUITE(logitem)

BOOST_AUTO_TEST_CASE(Test_LogItem_Data)
{
    const std::string source = "testSource";
    const std::string schemaAndData = "testSchemaAndData";

    try {
        DjsonLogItem item(source, schemaAndData);
        auto tag = item.GetTag();
        auto data = item.GetData();
        BOOST_REQUIRE_GT(tag.size(), 0);

        std::ostringstream expected;
        auto len = source.size() + schemaAndData.size() + tag.size() + 6;
        expected << len << "\n[\"" << source << "\"," << tag << "," << schemaAndData << "]";
        BOOST_CHECK_EQUAL(expected.str(), data);
    }
    catch(const std::exception & ex) {
        BOOST_FAIL("Test failed with unexpected exception: " << ex.what());
    }
}

BOOST_AUTO_TEST_CASE(Test_LogItem_CacheTime)
{
    const std::string source = "testSource";
    const std::string schemaAndData = "testSchemaAndData";

    try {
        DjsonLogItem item(source, schemaAndData);
        item.Touch();
        int nexpectedMS = 20;
        usleep(nexpectedMS*1000);
        auto actualMS = item.GetLastTouchMilliSeconds();
        BOOST_CHECK_LE(abs(actualMS - nexpectedMS), 1);
    }
    catch(const std::exception & ex) {
        BOOST_FAIL("Test failed with unexpected exception: " << ex.what());
    }
}

BOOST_AUTO_TEST_CASE(Test_IdMgr_BVT)
{
    try {
        IdMgr m;

        IdMgr::value_type_t emptyResult;
        BOOST_CHECK_THROW(m.GetItem("", emptyResult), std::invalid_argument);

        const std::string key = "abc";
        const std::string testValue = "testvalue";

        IdMgr::value_type_t value = std::make_pair(123, testValue);
        auto valueCopy = value;

        BOOST_CHECK_EQUAL(false, m.GetItem(key, value));
        BOOST_CHECK_EQUAL(true, valueCopy == value);

        // SaveItem won't create new id if key already exists
        for (int i = 0; i < 3; i++) {
            auto id1 = m.FindOrInsert(key, testValue);
            BOOST_CHECK_EQUAL(1, id1);
        }

        IdMgr::value_type_t newValue;
        BOOST_CHECK_EQUAL(true, m.GetItem(key, newValue));
        BOOST_CHECK_EQUAL(1, newValue.first);
        BOOST_CHECK_EQUAL(testValue, newValue.second);

        const std::string key2 = "def";
        m.Insert(key2, newValue);

        IdMgr::value_type_t setValue;
        BOOST_CHECK_EQUAL(true, m.GetItem(key2, setValue));
        BOOST_CHECK_EQUAL(true, setValue == newValue);
    }
    catch(const std::exception & ex) {
        BOOST_FAIL("Test failed with unexpected exception: " << ex.what());
    }
}

static void
TestEtwLogItem()
{
    EtwLogItem etwlog("testsource", "testguid", 123);

    etwlog.AddData("bool_true", true);
    etwlog.AddData("bool_false", false);

    etwlog.AddData("int32_data", 1);
    int64_t n = ((int64_t)1) << 40;
    etwlog.AddData("int64_data", n);

    etwlog.AddData("double_data", 0.0000004);
    etwlog.AddData("time_data", 11, 22);

    etwlog.AddData("charstr_data", "charstr");
    std::string str = "std::string";
    etwlog.AddData("stdstr_data", str);

    const std::string expectedData = R"([["EventId","FT_INT32"],["GUID","FT_STRING"],["bool_false","FT_BOOL"],["bool_true","FT_BOOL"],["charstr_data","FT_STRING"],["double_data","FT_DOUBLE"],["int32_data","FT_INT32"],["int64_data","FT_INT64"],["stdstr_data","FT_STRING"],["time_data","FT_TIME"]],[123,"testguid",false,true,"charstr",4e-07,1,1099511627776,"std::string",[11,22]]])";

    std::string actualData = etwlog.GetData();

    BOOST_CHECK_EQUAL(true, actualData.find(expectedData) != std::string::npos);
}


BOOST_AUTO_TEST_CASE(Test_EtwLogItem_BVT)
{
    try {
        TestEtwLogItem();
    }
    catch(const std::exception & ex) {
        BOOST_FAIL("Test failed with unexpected exception: " << ex.what());
    }
}

static void
CreateEtwLogItems(size_t nitems)
{
    for (size_t i = 0; i < nitems; i++) {
        EtwLogItem etwlog("testsource", "testguid", 123);

        etwlog.AddData("bool_true", true);
        etwlog.AddData("bool_false", false);

        etwlog.AddData("int32_data", (int64_t) i);
        etwlog.AddData("int64_data", (int64_t) i);

        etwlog.AddData("double_data", 0.0000004);
        etwlog.AddData("time_data", 11, 22);

        etwlog.AddData("charstr_data", "charstr");
        std::string str = "std::string " + std::to_string(i);
        etwlog.AddData("stdstr_data", str);

        (void)etwlog.GetData();
    }
}

#if 0
// Used to tune EtwLogItem related perf
BOOST_AUTO_TEST_CASE(Test_EtwLogItem_Perf)
{
    try {
        CreateEtwLogItems(10000);
    }
    catch(const std::exception & ex) {
        BOOST_FAIL("Test failed with unexpected exception: " << ex.what());
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
