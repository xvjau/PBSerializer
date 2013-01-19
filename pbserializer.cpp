#include "pbserializer.h"
#include <google/protobuf/message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <boost/concept_check.hpp>

#include <stack>
#include <iostream>
#include <cstdio>

namespace google {
namespace protobuf {

typedef std::vector<const FieldDescriptor*> FieldVector;

static short UTF8len(char b)
{
    short bits = 1;
    for (; (b & (1 << 8)) != 0; b <<= 1)
    {
        bits++;
    }
    return bits;
}

class JSONParser
{
private:
    class JSONContext
    {
    public:
        JSONContext(std::string _name):
            name(_name),
            inValue(false),
            inArray(false)
        {
        }

        std::string name;
        bool inValue;
        bool inArray;
    };
    std::stack<JSONContext> m_jsonContext;

    std::string getTempString()
    {
        std::size_t start = temp.find("\"");
        std::size_t end = temp.rfind("\"");

        std::string result;

        if (start != std::string::npos && end != std::string::npos)
        {
            result = temp.substr(start + 1, end - start - 1);
        }
        else
        {
            result = temp;
            if (result.compare("null") == 0)
            {
                result.clear();
            }
        }

        temp.clear();
        return result;
    }

    void checkValue()
    {
        if (m_jsonContext.top().inValue)
        {
            value(m_jsonContext.top().name, getTempString());
            m_jsonContext.top().inValue = false;
        }
        else if (m_jsonContext.top().inArray)
        {
            value(m_jsonContext.top().name, getTempString());
        }
    }

    std::istream    &data;
    std::size_t     dataRead;

    bool            aborted;
    bool            inQuotes;

    std::string     temp;

public:
    JSONParser(std::istream &_data):
        data(_data),
        dataRead(0),
        aborted(false),
        inQuotes(false)
    {
    }

    bool parse()
    {
        m_jsonContext.push(JSONContext("<root>"));

        try
        {
            std::string c;
            char highChar = 0;

            const std::size_t bufferSize = 2048;
            char buffer[bufferSize];

            std::size_t read;

            while((!aborted) && (!data.eof()))
            {
                data.read(buffer, bufferSize);
                read = data.gcount();

                dataRead += read;

                for(std::size_t i = 0; i != read && !aborted; i++)
                {
                    char b = buffer[i];

                    if(highChar == 0)
                    {
                        if(UTF8len(b) == 1)
                        {
                            highChar = 0;
                            c = (char) b;
                        }
                        else
                        {
                            highChar = b;
                            continue;
                        }
                    }
                    else
                    {
                        char utf8[2] = { highChar, b };
                        c = utf8;
                        highChar = 0;
                    }

                    if(c[0] == '"')
                    {
                        int pos = temp.length() - 1;

                        if(pos == -1 || temp[pos] != '\\')
                        {
                            inQuotes = !inQuotes;
                        }
                    }

                    if(inQuotes)
                    {
                        temp.append(c);
                    }
                    else
                    {
                        switch(c[0])
                        {
                            case '{':
                                m_jsonContext.top().inValue = false;
                                onStartElement(m_jsonContext.top().name);

                                m_jsonContext.push(JSONContext(m_jsonContext.top().name));
                                temp.clear();

                                break;

                            case '}':
                                checkValue();

                                if(m_jsonContext.top().inArray)
                                {
                                    onEndElement(m_jsonContext.top().name);
                                }
                                else
                                {
                                    onEndElement(m_jsonContext.top().name);
                                }

                                m_jsonContext.pop();
                                break;

                            case '[':
                                m_jsonContext.top().inValue = false;
                                m_jsonContext.top().inArray = true;
                                temp.clear();
                                onStartArray(m_jsonContext.top().name);
                                break;

                            case ']':
                                checkValue();
                                m_jsonContext.top().inArray = false;
                                onEndArray(m_jsonContext.top().name);
                                break;

                            case ':':
                                m_jsonContext.top().name = getTempString();
                                m_jsonContext.top().inValue = true;
                                break;

                            case ',':
                                checkValue();
                                break;

                            default:
                                temp.append(c);
                        }
                    }
                }
            }

            return true;
        }
        catch(const std::exception& e)
        {
            // TODO Auto-generated catch block
            std::cerr << "JSON Parse error: " << e.what() << std::endl;
        }

        return false;
    }

protected:
    virtual void onStartElement(std::string name) = 0;
    virtual void onEndElement(std::string name) = 0;
    virtual void onStartArray(std::string name) = 0;
    virtual void onEndArray(std::string name) = 0;
    virtual void value(std::string name, std::string value) = 0;
};

class JSONPBParser: public JSONParser
{
private:
    class PBContext
    {
    public:
        PBContext()
        {
            message = NULL;
            refl = NULL;
            desc = NULL;
        }

        PBContext(Message* _message):
            message(_message)
        {
            refl = message->GetReflection();
            desc = message->GetDescriptor();
        }

        Message*                message;
        const Reflection*       refl;
        const Descriptor*       desc;
    };
    std::stack<PBContext>   m_pbContext;

public:
    JSONPBParser(Message* message, std::istream& jsonData):
        JSONParser(jsonData)
    {
        m_pbContext.push(message);
    }

protected:
    virtual void onStartElement(std::string name)
    {
        Message* message = m_pbContext.top().message;
        const Reflection* refl = m_pbContext.top().refl;
        const FieldDescriptor* currentField = message->GetDescriptor()->FindFieldByName(name);

        if (currentField)
        {
            if (!currentField->is_repeated())
                m_pbContext.push(refl->MutableMessage(message, currentField));
            else
                m_pbContext.push(refl->AddMessage(message, currentField));
        }
        else
        {
            if (name.compare("<root>") != 0)
                m_pbContext.push(PBContext());
        }
    }

    virtual void onEndElement(std::string name)
    {
        m_pbContext.pop();
    }

    virtual void onStartArray(std::string name)
    {
    }

    virtual void onEndArray(std::string name)
    {
    }

    virtual void value(std::string name, std::string value)
    {
        if (m_pbContext.top().message)
        {
            const Reflection* refl = m_pbContext.top().refl;
            Message* message = m_pbContext.top().message;
            const FieldDescriptor* currentField = message->GetDescriptor()->FindFieldByName(name);

            if (!currentField)
            {
                std::cerr << "Unknown field: '" << name << "'\n";
            }
            else
            {
                switch(currentField->cpp_type())
                {
                    case FieldDescriptor::CPPTYPE_INT32:   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
                    {
                        int val = atoi(value.c_str());

                        if (!currentField->is_repeated())
                            refl->SetInt32(message, currentField, val);
                        else
                            refl->AddInt32(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64:   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
                    {
                        long long val = atoll(value.c_str());

                        if (!currentField->is_repeated())
                            refl->SetInt64(message, currentField, val);
                        else
                            refl->AddInt64(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
                    {
                        int val = atoi(value.c_str());

                        if (!currentField->is_repeated())
                            refl->SetUInt32(message, currentField, val);
                        else
                            refl->AddUInt32(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
                    {
                        long long val = atoll(value.c_str());

                        if (!currentField->is_repeated())
                            refl->SetUInt64(message, currentField, val);
                        else
                            refl->AddUInt64(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
                    {
                        double val = strtod(value.c_str(), NULL);

                        if (!currentField->is_repeated())
                            refl->SetDouble(message, currentField, val);
                        else
                            refl->AddDouble(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT:   // TYPE_FLOAT
                    {
                        float val = strtof(value.c_str(), NULL);

                        if (!currentField->is_repeated())
                            refl->SetFloat(message, currentField, val);
                        else
                            refl->AddFloat(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL:    // TYPE_BOOL
                    {
                        bool val = (strcasecmp(value.c_str(), "true") == 0) ||
                                    (strcasecmp(value.c_str(), "t") == 0) ||
                                    (strcasecmp(value.c_str(), "1") == 0);

                        if (!currentField->is_repeated())
                            refl->SetBool(message, currentField, val);
                        else
                            refl->AddBool(message, currentField, val);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM:    // TYPE_ENUM
                    {
                        int val = atoll(value.c_str());

                        if (!currentField->is_repeated())
                            refl->SetEnum(message, currentField, currentField->enum_type()->FindValueByNumber(val));
                        else
                            refl->AddEnum(message, currentField, currentField->enum_type()->FindValueByNumber(val));
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
                    {
                        if (!currentField->is_repeated())
                            refl->SetString(message, currentField, value);
                        else
                            refl->AddString(message, currentField, value);
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: // TYPE_MESSAGE, TYPE_GROUP
                    {
                        //TODO WTF?
                        break;
                    }
                }
            }
        }
    }
};

void ParseJSON(google::protobuf::Message* message, std::istream& jsonData)
{
    JSONPBParser parser(message, jsonData);
    parser.parse();
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

}
}
