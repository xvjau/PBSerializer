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

    boost::property_tree::ptree SerializePtree() const
    {
        boost::property_tree::ptree result;

        SerializePtree(*this, &result, this->GetTypeName() + '.');

        return result;
    }

    static void SerializePtree(const Message& message, boost::property_tree::ptree* result, std::string path)
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
                        SerializePtree(refl->GetMessage(message, desc), result, p + '.');
                        break;
                    }
                }
            }
            else // is_repeated
            {
                #warning FIXME This does not work yet!
                const int count = refl->FieldSize(message, desc);

                switch(desc->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        #warning FIXME This does not work yet!
                        ptree array;

                        for(int i = 0; i != count; i++)
                        {
                            ptree item;
                            item.put("", refl->GetRepeatedInt32(message, desc, i));
                            array.push_back(std::make_pair("item", item));
                        }

                        result->add_child(p, array);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedInt64(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedUInt32(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedUInt64(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedDouble(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedFloat(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedBool(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedEnum(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedString(message, desc, i));
                        break;

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO How to do this?
                        assert(false);

                        SerializePtree(refl->GetMessage(message, desc), result, p + '.');
                        for(int i = 0; i != count; i++)
                            result->add(p, refl->GetRepeatedInt32(message, desc, i));
                        break;
                    }
                }
            }
        }
    }

public:
    bool SerializeJsonToOStream(std::ostream* output) const
    {
        boost::property_tree::write_json(*output, SerializePtree());
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
        boost::property_tree::write_xml(*output, SerializePtree());
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
        boost::property_tree::write_ini(*output, SerializePtree());
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
        boost::property_tree::write_info(*output, SerializePtree());
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
