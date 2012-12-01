#include <iostream>

#include "message.pb.h"
#include <pbserializer.h>

using ::std::string;
using namespace ::google::protobuf;

typedef PBSerializer<test_message> test_message_json;

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    test_message_json msg;

    msg.set_id(42);
    msg.set_message("Hello World!");

    date_time* dt = msg.mutable_sub_message();

    dt->set_day(31);
    dt->set_month(02);
    dt->set_year(1972);

    RepeatedField<int32>* arr = msg.mutable_int_array();

    arr->Add(1);
    arr->Add(1);
    arr->Add(2);
    arr->Add(3);
    arr->Add(5);
    arr->Add(8);
    arr->Add(13);
    arr->Add(21);

    string json, xml, ini, info;

    msg.SerializeJsonToString(&json);
    msg.SerializeXmlToString(&xml);
    //msg.SerializeIniToString(&ini);
    msg.SerializeInfoToString(&info);

    std::cout << "pb=\"" << msg.DebugString() << "\"\n\n";
    std::cout << "json=\"" << json << "\"\n\n";
    std::cout << "xml=\"" << xml << "\"\n\n";
    std::cout << "ini=\"" << ini << "\"\n\n";
    std::cout << "info=\"" << info << "\"\n" << std::endl;

    return 0;
}
