/*
    Protobuf Serializer using boost's property tree
    Copyright (C) 2012  Gianni Rossi <gianni.rossi@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PBSERIALIZER_H
#define PBSERIALIZER_H
#include <map>
#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace google {
namespace protobuf {

template<class T>
class PBSerializer : public T
{
protected:
    typedef std::vector<const FieldDescriptor*> FieldVector;

    boost::property_tree::ptree SerializePtree(bool useArrayItemNames) const
    {
        boost::property_tree::ptree result;

        SerializePtree(*this, &result, std::string(""), useArrayItemNames);

        // This is a bit reduntant, since most compilers will do the RVO as below:
        // http://en.wikipedia.org/wiki/Return_value_optimization
#if (__cplusplus >= 201103L)
        return std::move(result);
#else
        return result;
#endif
    }

    static void ParsePtree(Message* message, boost::property_tree::ptree* p)
    {
        using namespace boost::property_tree;
        
        const Reflection* refl = message->GetReflection();
        const Descriptor* mDesc = message->GetDescriptor();
        
        ptree::iterator it = p->begin(), end = p->end();

        for(; it != end; it++)
        {
            const FieldDescriptor* desc = mDesc->FindFieldByName(it->first);
            std::cout << it->first << "," << it->second.get<std::string>("") << std::endl;
            
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

    static void SerializePtree(const Message& message, boost::property_tree::ptree* result, std::string path, bool useArrayItemNames)
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
                        SerializePtree(refl->GetMessage(message, desc), result, p + '.', useArrayItemNames);
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
                            SerializePtree(refl->GetRepeatedMessage(message, desc, i), &item, "", useArrayItemNames);
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

public:
    
    bool ParseJsonFromString(std::string input)
    {
        std::stringstream str;
        str << input;
        boost::property_tree::ptree p;
        boost::property_tree::read_json(str,p);
        ParsePtree(this,&p);
        return true;
    }
    
    bool SerializeJsonToOStream(std::ostream* output) const
    {
        boost::property_tree::write_json(*output, SerializePtree(false));
        return true;
    }

    bool SerializeJsonToString(std::string* output) const
    {
        std::stringstream str;
        SerializeJsonToOStream(&str);
        output->assign(str.str());
        return true;
    }

    bool SerializeXmlToOStream(std::ostream* output) const
    {
        boost::property_tree::write_xml(*output, SerializePtree(true));
        return true;
    }

    bool SerializeXmlToString(std::string* output) const
    {
        std::stringstream str;
        SerializeXmlToOStream(&str);
        output->assign(str.str());
        return true;
    }

    bool SerializeIniToOStream(std::ostream* output) const
    {
        boost::property_tree::write_ini(*output, SerializePtree(true));
        return true;
    }

    bool SerializeIniToString(std::string* output) const
    {
        std::stringstream str;
        SerializeIniToOStream(&str);
        output->assign(str.str());
        return true;
    }

    bool SerializeInfoToOStream(std::ostream* output) const
    {
        boost::property_tree::write_info(*output, SerializePtree(false));
        return true;
    }

    bool SerializeInfoToString(std::string* output) const
    {
        std::stringstream str;
        SerializeInfoToOStream(&str);
        output->assign(str.str());
        return true;
    }
};

} // namespace protobuf
} // namespace google

#endif // PBSERIALIZER_H
