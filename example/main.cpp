#include <iostream>

#include "message.pb.h"
#include <pbserializer.h>

using std::string;

typedef PBSerializer<test_message> test_message_json;

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    test_message_json msg;

    msg.set_id(42);
    msg.set_message("Hello World!");

    auto dt = msg.mutable_sub_message();

    dt->set_day(31);
    dt->set_month(02);
    dt->set_year(1972);

    auto arr = msg.mutable_int_array();

    arr->Add(1);
    arr->Add(1);
    arr->Add(2);
    arr->Add(3);
    arr->Add(5);
    arr->Add(8);
    arr->Add(13);
    arr->Add(21);

    string json, xml;

    msg.SerializeJsonToString(&json);
    msg.SerializeXmlToString(&xml);

    std::cout << "pb=\"" << msg.DebugString() << '"' << std::endl;
    std::cout << "json=\"" << json << '"' << std::endl;
    std::cout << "xml=\"" << xml << '"' << std::endl;

    return 0;
}
