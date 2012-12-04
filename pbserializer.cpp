#include "pbserializer.h"
#include <google/protobuf/message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <boost/property_tree/ptree.hpp>

namespace google {
namespace protobuf {

typedef std::vector<const FieldDescriptor*> FieldVector;

void ParsePtree(Message* message, boost::property_tree::ptree* p)
{
    using namespace boost::property_tree;

    const Reflection* refl = message->GetReflection();
    const Descriptor* mDesc = message->GetDescriptor();

    ptree::iterator it = p->begin(), end = p->end();

    for(; it != end; it++)
    {
        const FieldDescriptor* desc = mDesc->FindFieldByName(it->first);
        
        if (!desc->is_repeated())
        {
            switch(desc->cpp_type())
            {
                case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    refl->SetInt32(message, desc, it->second.get<int>(""));
                    break;

                case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    refl->SetInt64(message, desc, it->second.get<int>(""));
                    break;

                case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    refl->SetUInt32(message, desc, it->second.get<int>(""));
                    break;

                case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    refl->SetUInt64(message, desc, it->second.get<int>(""));
                    break;

                case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    refl->SetDouble(message, desc, it->second.get<double>(""));
                    break;

                case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    refl->SetFloat(message, desc, it->second.get<float>(""));
                    break;

                case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    refl->SetBool(message, desc, it->second.get<bool>(""));
                    break;

                case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    refl->SetEnum(message, desc, desc->enum_type()->FindValueByNumber(it->second.get<int>("")));
                    break;

                case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    refl->SetString(message, desc, it->second.get<std::string>(""));
                    break;

                case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                {
                    ParsePtree(refl->MutableMessage(message, desc), &(it->second));
                    break;
                }

            }
        }
        else // is_repeated
        {
            ptree::iterator i3 = it->second.begin(), e3 = it->second.end();
            for(; i3 != e3; i3++)
            {
                switch(desc->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                        refl->AddInt32(message, desc, i3->second.get<int>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                        refl->AddInt64(message, desc, i3->second.get<int>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                        refl->AddUInt32(message, desc, i3->second.get<int>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                        refl->AddUInt64(message, desc, i3->second.get<int>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                        refl->AddDouble(message, desc, i3->second.get<double>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                        refl->AddFloat(message, desc, i3->second.get<float>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                        refl->AddBool(message, desc, i3->second.get<bool>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                        refl->AddEnum(message, desc, desc->enum_type()->FindValueByNumber(i3->second.get<int>("")));
                        break;

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                        refl->AddString(message, desc, i3->second.get<std::string>(""));
                        break;

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        #warning Check this!
                        Message* m = refl->AddMessage(message, desc);
                        ParsePtree(m, &(i3->second));
                        break;
                    }
                }
            }
        }

    }

}

void SerializePtreeFromMessage(const Message& message, boost::property_tree::ptree* result, std::string path, bool useArrayItemNames)
{
    using boost::property_tree::ptree;

    const Reflection* refl = message.GetReflection();

    FieldVector fields;

    refl->ListFields(message, &fields);

    FieldVector::iterator it = fields.begin(), end = fields.end();
    for(; it != end; it++)
    {
        const FieldDescriptor* desc = *it;

        std::string p = path + desc->name();

        if (!desc->is_repeated())
        {
            switch(desc->cpp_type())
            {
                case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    result->put(p, refl->GetInt32(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    result->put(p, refl->GetInt64(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    result->put(p, refl->GetUInt32(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    result->put(p, refl->GetUInt64(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    result->put(p, refl->GetDouble(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    result->put(p, refl->GetFloat(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    result->put(p, refl->GetBool(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    result->put(p, refl->GetEnum(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    result->put(p, refl->GetString(message, desc));
                    break;

                case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                {
                    SerializePtreeFromMessage(refl->GetMessage(message, desc), result, p + '.', useArrayItemNames);
                    break;
                }
            }
        }
        else // is_repeated
        {
            const int count = refl->FieldSize(message, desc);

            #warning FIXME This does not work yet for all cases (only XML and JSON)
            const std::string itemName = useArrayItemNames ? desc->name() : std::string();

            ptree array;

            for(int i = 0; i != count; i++)
            {
                ptree item;

                switch(desc->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                        item.put("", refl->GetRepeatedInt32(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                        item.put("", refl->GetRepeatedInt64(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                        item.put("", refl->GetRepeatedUInt32(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                        item.put("", refl->GetRepeatedUInt64(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                        item.put("", refl->GetRepeatedDouble(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                        item.put("", refl->GetRepeatedFloat(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                        item.put("", refl->GetRepeatedBool(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                        item.put("", refl->GetRepeatedEnum(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                        item.put("", refl->GetRepeatedString(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        #warning Check this!
                        SerializePtreeFromMessage(refl->GetRepeatedMessage(message, desc, i), &item, "", useArrayItemNames);
                        break;
                    }
                }
#if (__cplusplus >= 201103L)
                array.push_back(std::make_pair(itemName, std::move(item)));
#else
                array.push_back(std::make_pair(itemName, item));
#endif
            }

#if (__cplusplus >= 201103L)
            result->add_child(p, std::move(array));
#else
            result->add_child(p, array);
#endif
        }
    }
}

}
}