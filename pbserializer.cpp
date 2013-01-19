#include "pbserializer.h"
#include <google/protobuf/message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <boost/concept_check.hpp>

#include <stack>
#include <iostream>
#include <cstdio>

#include "JSON_parser/JSON_parser.h"

namespace google {
namespace protobuf {

typedef std::vector<const FieldDescriptor*> FieldVector;

#if (__cplusplus >= 201103L)
constexpr
#endif
    static short UTF8len(char b)
{
    short bits = 1;
    for (; (b & (1 << 8)) != 0; b <<= 1)
    {
        bits++;
    }
    return bits;
}

class JSONParserCtx
{
public:
    JSON_parser         m_parser;

    Message*            m_message;

    const Reflection*   m_refl;
    const Descriptor*   m_desc;
    const FieldDescriptor*      m_currentField;

    std::istream&       m_jsonData;

    JSONParserCtx(Message* message, std::istream& jsonData):
        m_message(message),
        m_jsonData(jsonData)
    {
        m_refl = message->GetReflection();
        m_desc = message->GetDescriptor();
        m_currentField = NULL;

        JSON_config config;
        memset(&config, 0, sizeof(JSON_config));

        config.callback = _parserCallback;
        config.callback_ctx = this;

        m_parser = new_JSON_parser(&config);
    }

    ~JSONParserCtx()
    {
        delete_JSON_parser(m_parser);
    }

    void parse()
    {
        char tailChar = 0;

        while(!m_jsonData.eof())
        {
            const std::size_t bufferSize = 2048;
            char buffer[bufferSize];

            char *bufferStart = tailChar ? &buffer[1] : &buffer[0];

            m_jsonData.read(bufferStart, tailChar ? bufferSize - 1 : bufferSize);
            std::size_t read = m_jsonData.gcount();

            for(uint i = 0; i < read; /* no inc */)
            {
                const char c = buffer[i];

                switch(UTF8len(c))
                {
                    case 0: assert(UTF8len(c) != 0);  // Should NEVER happen
                    case 1:
                        i++;
                        JSON_parser_char(m_parser, c);
                        break;
                    case 2:
                    {
                        if (i < read - 1)
                        {
                            i += 2;
                            short *str = reinterpret_cast<short*>(buffer[i]);
                            JSON_parser_char(m_parser, *str);

                        }
                        else
                        {
                            tailChar = buffer[i];
                            goto EXIT_FOR;
                        }
                        break;
                    }
                    case 3:
                    case 4:
                    {
                        i += UTF8len(c);
                        int *str = reinterpret_cast<int*>(buffer[i]);
                        JSON_parser_char(m_parser, *str);
                        break;
                    }
                }
            }

            EXIT_FOR: {}
        }
    }

    static int _parserCallback(void* ctx, int type, const struct JSON_value_struct* value)
    {
        return static_cast<JSONParserCtx*>(ctx)->parserCallback(type, value);
    }

    int parserCallback(int type, const struct JSON_value_struct* value)
    {
        switch( type )
        {
            case JSON_T_ARRAY_BEGIN:
            case JSON_T_OBJECT_BEGIN:
            {
                //thiz->elementStart( nullptr );
                break;
            }

            case JSON_T_ARRAY_END:
            case JSON_T_OBJECT_END:
            {
                //thiz->elementEnd();
                break;
            }

            case JSON_T_INTEGER:
            {
                switch(m_currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        setInt32(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        setInt64(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        setUInt32(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        setInt64(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        setDouble(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        setFloat(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        setBool(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        setEnum(value->vu.integer_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        char buffer[64];
                        snprintf(buffer, 64, "%ld", value->vu.integer_value);
                        setString(buffer);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO wtf?
                        break;
                    }
                }
                break;
            }

            case JSON_T_FLOAT:
            {
                switch(m_currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        setInt32(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        setInt64(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        setUInt32(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        setInt64(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        setDouble(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        setFloat(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        setBool(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        setEnum(value->vu.float_value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        char buffer[64];
                        snprintf(buffer, 64, "%f", value->vu.float_value);
                        setString(buffer);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO wtf?
                        break;
                    }
                }
                break;
            }

            case JSON_T_NULL:
                break;

            case JSON_T_TRUE:
            {
                switch(m_currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        setInt32(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        setInt64(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        setUInt32(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        setInt64(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        setDouble(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        setFloat(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        setBool(true);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        setEnum(1);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        setString("true");
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO wtf?
                        break;
                    }
                }
                break;
            }

            case JSON_T_FALSE:
            {
                switch(m_currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        setInt32(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        setInt64(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        setUInt32(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        setInt64(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        setDouble(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        setFloat(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        setBool(false);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        setEnum(0);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        setString("false");
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO wtf?
                        break;
                    }
                }
                break;
            }

            case JSON_T_STRING:
            {
                std::string str = std::string( value->vu.str.value, value->vu.str.length );

                switch(m_currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        int val = atoi(str.c_str());
                        setInt32(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        long long val = atoll(str.c_str());
                        setInt64(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        int val = atoi(str.c_str());
                        setUInt32(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        long long val = atoll(str.c_str());
                        setInt64(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        double val = strtod(str.c_str(), NULL);
                        setDouble(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        float val = strtof(str.c_str(), NULL);
                        setFloat(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        bool val = (strcasecmp(str.c_str(), "true") == 0) ||
                                    (strcasecmp(str.c_str(), "t") == 0) ||
                                    (strcasecmp(str.c_str(), "1") == 0);
                        setBool(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        int val = atoll(str.c_str());
                        setEnum(val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        setString(str);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        // TODO wtf?
                        break;
                    }
                }
                break;
            }

            case JSON_T_KEY:
            {
                std::string fieldName = std::string(value->vu.str.value, value->vu.str.length);
                m_currentField = m_desc->FindFieldByName(fieldName);
                break;
            }

            default:
            {
                assert( false );
            }
        }

        return 1; // continue parsing
    }

    void setInt32(int32_t val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetInt32(m_message, m_currentField, val);
        else
            m_refl->AddInt32(m_message, m_currentField, val);
    }
    void setInt64(int64_t val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetInt64(m_message, m_currentField, val);
        else
            m_refl->AddInt64(m_message, m_currentField, val);
    }
    void setUInt32(uint32_t val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetUInt32(m_message, m_currentField, val);
        else
            m_refl->AddUInt32(m_message, m_currentField, val);
    }
    void setUInt64(uint64_t val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetUInt64(m_message, m_currentField, val);
        else
            m_refl->AddUInt64(m_message, m_currentField, val);
    }
    void setDouble(double val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetDouble(m_message, m_currentField, val);
        else
            m_refl->AddDouble(m_message, m_currentField, val);
    }
    void setFloat(float val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetFloat(m_message, m_currentField, val);
        else
            m_refl->AddFloat(m_message, m_currentField, val);
    }
    void setBool(bool val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetBool(m_message, m_currentField, val);
        else
            m_refl->AddBool(m_message, m_currentField, val);
    }
    void setEnum(int val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetEnum(m_message, m_currentField, m_currentField->enum_type()->FindValueByNumber(val));
        else
            m_refl->AddEnum(m_message, m_currentField, m_currentField->enum_type()->FindValueByNumber(val));
    }
    void setString(std::string val)
    {
        if (!m_currentField->is_repeated())
            m_refl->SetString(m_message, m_currentField, val);
        else
            m_refl->AddString(m_message, m_currentField, val);
    }
};

void ParseJSON(google::protobuf::Message* message, std::istream& jsonData)
{
    JSONParserCtx parser(message, jsonData);
    parser.parse();

//     case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
//     {
//         ParsePtree(refl->MutableMessage(m_message, m_desc), &(it->second));
//         break;
//     }
//
//     case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
//     {
//         #warning Check this!
//         Message* m = refl->AddMessage(m_message, m_desc);
//         ParsePtree(m, &(i3->second));
//         break;
//     }

}

class JSONWriterCtx
{
private:
    std::ostream&   m_jsonData;

    inline void checkFirst(bool &isFirst)
    {
        if (isFirst)
            isFirst = false;
        else
            m_jsonData << ',';
    }

    inline void checkPath(const std::string& path)
    {
        if (!path.empty())
            m_jsonData << '"' << path << "\":";
    }

    void put(bool &isFirst, const std::string& path, const std::string value)
    {
        checkFirst(isFirst);
        checkPath(path);
        m_jsonData << '"' << value << '"';
    }

    void put(bool &isFirst, const std::string& path, const bool value)
    {
        checkFirst(isFirst);
        checkPath(path);
        m_jsonData << (value ? "true" : "false");
    }

    template<typename T>
    void put(bool &isFirst, const std::string& path, T value)
    {
        checkFirst(isFirst);
        checkPath(path);
        m_jsonData << value;
    }

public:
    JSONWriterCtx(std::ostream& jsonData):
        m_jsonData(jsonData)
    {
    }

    void write(const Message&  message)
    {
        bool isFirst = true;

        const Reflection*   refl = message.GetReflection();
        FieldVector         fields;

        refl->ListFields(message, &fields);

        m_jsonData << '{';

        FieldVector::iterator it = fields.begin(), end = fields.end();
        for(; it != end; it++)
        {
            const FieldDescriptor* desc = *it;

            if (!desc->is_repeated())
            {
                switch(desc->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                        put(isFirst, desc->name(), refl->GetInt32(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                        put(isFirst, desc->name(), refl->GetInt64(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                        put(isFirst, desc->name(), refl->GetUInt32(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                        put(isFirst, desc->name(), refl->GetUInt64(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                        put(isFirst, desc->name(), refl->GetDouble(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                        put(isFirst, desc->name(), refl->GetFloat(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                        put(isFirst, desc->name(), refl->GetBool(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                        put(isFirst, desc->name(), refl->GetEnum(message, desc)->number());
                        break;

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                        put(isFirst, desc->name(), refl->GetString(message, desc));
                        break;

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        checkFirst(isFirst);
                        checkPath(desc->name());
                        write(refl->GetMessage(message, desc));
                        break;
                    }
                }
            }
            else // is_repeated
            {
                const int count = refl->FieldSize(message, desc);

                if (count)
                {
                    if (!isFirst)
                    {
                        m_jsonData << ',';
                        isFirst = true;
                    }

                    checkPath(desc->name());

                    m_jsonData << '[';

                    for(int i = 0; i != count; i++)
                    {
                        switch(desc->cpp_type())
                        {
                            case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                                put(isFirst, "", refl->GetRepeatedInt32(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                                put(isFirst, "", refl->GetRepeatedInt64(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                                put(isFirst, "", refl->GetRepeatedUInt32(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                                put(isFirst, "", refl->GetRepeatedUInt64(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                                put(isFirst, "", refl->GetRepeatedDouble(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                                put(isFirst, "", refl->GetRepeatedFloat(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                                put(isFirst, "", refl->GetRepeatedBool(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                                put(isFirst, "", refl->GetRepeatedEnum(message, desc, i)->number());
                                break;

                            case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                                put(isFirst, "", refl->GetRepeatedString(message, desc, i));
                                break;

                            case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                            {
                                checkFirst(isFirst);
                                write(refl->GetRepeatedMessage(message, desc, i));
                                break;
                            }
                        }
                    }

                    m_jsonData << ']';
                }
            }

        }
        m_jsonData << '}';
    }
};

void SerializeJSON(const Message& message, std::ostream& jsonData)
{
    JSONWriterCtx writer(jsonData);
    writer.write(message);
}

/*
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
                    result->put(p, refl->GetEnum(message, desc)->number());
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
                        item.put("", refl->GetRepeatedEnum(message, desc, i)->number());
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
}*/

}
}
