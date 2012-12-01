#include <iostream>

#include "message.pb.h"
#include <pbserializer.h>

// Use C++11 to get performance mesures
#if (__cplusplus >= 201103L)
#include <chrono>
using namespace std::chrono;

#define INIT_TIMER typedef high_resolution_clock __hrc; auto __hrc_start = __hrc::now()
#define SET_TIMER(NAME) auto __hrc_timer_ ## NAME ## _ = duration_cast<duration<double>>(__hrc::now() - __hrc_start); __hrc_start = __hrc::now()

#define GET_TIME(STR, NAME)                                                             \
do {                                                                                    \
    std::cout << STR << __hrc_timer_ ## NAME ## _.count() << "\n";                                   \
} while (false)
#else
#define INIT_TIMER          do { } while (false)
#define SET_TIMER(NAME)     do { } while (false)
#define GET_TIME(STR, NAME) do { } while (false)
#endif


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
    i->set_text("two");
    i->set_number(2);
    i->set_float_number(0.2);

    i = obj_arry->Add();
    i->set_text("Three");
    i->set_number(3);
    i->set_float_number(0.3);

    string pb, json, xml, ini, info;

    INIT_TIMER;

    pb = msg.DebugString();                         SET_TIMER(pb);
    msg.SerializeJsonToString(&json);               SET_TIMER(json);
    msg.SerializeXmlToString(&xml);                 SET_TIMER(xml);
    //msg.SerializeIniToString(&ini);               SET_TIMER(ini);
    msg.SerializeInfoToString(&info);               SET_TIMER(info);

    std::cout << "pb=\"" << pb << "\"\n\n";
    std::cout << "json=\"" << json << "\"\n\n";
    std::cout << "xml=\"" << xml << "\"\n\n";
    //std::cout << "ini=\"" << ini << "\"\n\n";
    std::cout << "info=\"" << info << "\"\n" << std::endl;

    GET_TIME("PB = ", pb);
    GET_TIME("JSON = ", json);
    GET_TIME("XML = ", xml);
    //GET_TIME("INI = ", ini);
    GET_TIME("INFO = ", info);

    return 0;
}
