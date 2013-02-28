#include <iostream>
#include <fstream>
#include <algorithm>
#include <gtest/gtest.h>
#include "message.pb.h"
#include <pbserializer.h>

using namespace ::google::protobuf;
using namespace std;
typedef PBSerializer<test_message> test_message_json;

TEST(PBSerializerTests, TestJsonSerialization)
{
    /**
     * This tests seems a bit fragile, is it really
     * a good idea comparing two strings so blindly ?
     */
    test_message_json msg;

    msg.set_id(42);
    msg.set_message("Hello World!");

    date_time* dt = msg.mutable_sub_message();

    dt->set_day(31);
    dt->set_month(02);
    dt->set_year(1972);

    RepeatedField<int32>* int_arr = msg.mutable_int_array();

    int_arr->Add(1);
    int_arr->Add(1);
    int_arr->Add(2);
    int_arr->Add(3);
    int_arr->Add(5);
    int_arr->Add(8);
    int_arr->Add(13);
    int_arr->Add(21);

    RepeatedPtrField<sub_array>* obj_arry = msg.mutable_obj_array();

    sub_array* i = obj_arry->Add();
    i->set_text("One");
    i->set_number(1);
    i->set_float_number(0.1);

    i = obj_arry->Add();
    i->set_text("\"two\"");
    i->set_number(2);
    i->set_float_number(0.2);

    i = obj_arry->Add();
    i->set_text("Three");
    i->set_number(3);
    i->set_float_number(0.3);

    string json;
    msg.SerializeJsonToString(json);

    string line;
    stringstream r;
    ifstream file ("example.json");
    if (file.is_open())
    {
        while ( file.good() )
        {
            getline (file,line);
            r << line << endl;
        }
        file.close();
    }
    
    // removing linebreaks and spaces
    string response = r.str();
    string::iterator end_pos = remove(response.begin(), response.end(), '\n');
    response.erase(end_pos, response.end());
    
    end_pos = remove(response.begin(), response.end(), ' ');
    response.erase(end_pos, response.end());
    
    end_pos = remove(json.begin(), json.end(), '\n');
    json.erase(end_pos, json.end());
    
    end_pos = remove(json.begin(), json.end(), ' ');
    json.erase(end_pos, json.end());
    
    ASSERT_STREQ(response.c_str(), json.c_str());
}

TEST(PBSerializerTests, TestJsonParsing)
{
    /**
     * This tests seems a bit fragile, is it really
     * a good idea comparing two strings so blindly ?
     */
    test_message_json msg;

    string line;
    stringstream r;
    ifstream file ("example.json");
    if (file.is_open())
    {
        while ( file.good() )
        {
            getline (file,line);
            r << line << endl;
        }
        file.close();
    }
    msg.ParseJsonFromString(r.str());

    ASSERT_EQ(msg.id(), 42);
    ASSERT_STREQ(msg.message().c_str(), "Hello World!");

    ASSERT_EQ(msg.sub_message().day(), 31);
    ASSERT_EQ(msg.sub_message().month(), 02);
    ASSERT_EQ(msg.sub_message().year(), 1972);

    ASSERT_EQ(msg.int_array_size(), 8);
    ASSERT_EQ(msg.int_array(0), 1);
    ASSERT_EQ(msg.int_array(1), 1);
    ASSERT_EQ(msg.int_array(2), 2);
    ASSERT_EQ(msg.int_array(3), 3);
    ASSERT_EQ(msg.int_array(4), 5);
    ASSERT_EQ(msg.int_array(5), 8);
    ASSERT_EQ(msg.int_array(6), 13);
    ASSERT_EQ(msg.int_array(7), 21);

    ASSERT_EQ(msg.obj_array_size(), 3);
    ASSERT_STREQ(msg.obj_array(0).text().c_str(), "One");
    ASSERT_EQ(msg.obj_array(0).number(), 1);
    ASSERT_NEAR(msg.obj_array(0).float_number(), 0.1, 0.01);

    ASSERT_STREQ(msg.obj_array(1).text().c_str(), "\"two\"");
    ASSERT_EQ(msg.obj_array(1).number(), 2);
    ASSERT_NEAR(msg.obj_array(1).float_number(), 0.2, 0.01);

    ASSERT_STREQ(msg.obj_array(2).text().c_str(), "Three");
    ASSERT_EQ(msg.obj_array(2).number(), 3);
    ASSERT_NEAR(msg.obj_array(2).float_number(), 0.3, 0.01);
}
