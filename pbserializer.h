/*
    Protobuf Serializer using boost's property tree
    Copyright (C) 2012
    Gianni Rossi <gianni.rossi@gmail.com>, Caio Casimiro <caiorcasimiro@gmail.com>

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
#include <sstream>

#include <istream>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

namespace google {
namespace protobuf {

void ParseJSON(Message* message, std::istream* jsonData);

template<class T>
class PBSerializer : public T
{
protected:
//     boost::property_tree::ptree SerializePtree(bool useArrayItemNames) const
//     {
//         boost::property_tree::ptree result;
//
//         SerializePtreeFromMessage(*this, &result, std::string(""), useArrayItemNames);
//
//         // This is a bit reduntant, since most compilers will do the RVO as below:
//         // http://en.wikipedia.org/wiki/Return_value_optimization
// #if (__cplusplus >= 201103L)
//         return std::move(result);
// #else
//         return result;
// #endif
//     }

public:

    bool ParseJsonFromString(std::string input)
    {
        std::stringstream str;
        str << input;
        ParseJSON(this, &str);
        return true;
    }

//     bool SerializeJsonToOStream(std::ostream* output) const
//     {
//         boost::property_tree::write_json(*output, SerializePtree(false));
//         return true;
//     }
//
//     bool SerializeJsonToString(std::string* output) const
//     {
//         std::stringstream str;
//         SerializeJsonToOStream(&str);
//         output->assign(str.str());
//         return true;
//     }
//
//     bool SerializeXmlToOStream(std::ostream* output) const
//     {
//         boost::property_tree::write_xml(*output, SerializePtree(true));
//         return true;
//     }
//
//     bool SerializeXmlToString(std::string* output) const
//     {
//         std::stringstream str;
//         SerializeXmlToOStream(&str);
//         output->assign(str.str());
//         return true;
//     }
};

} // namespace protobuf
} // namespace google

#endif // PBSERIALIZER_H
