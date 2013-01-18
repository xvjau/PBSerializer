// From Java:

class JSONParser
{
private:
    class Context
    {
    public:
        Context(std::string _name):
            name(_name),
            inValue(false),
            inArray(false)
        {
        }

        std::string name;
        bool inValue;
        bool inArray;
    };
    std::stack<Context> context;

    short UTF8len(char b)
    {
        short bits = 1;
        for (; (b & (1 << 8)) != 0; b <<= 1)
        {
            bits++;
        }
        return bits;
    }

    std::string getTempString()
    {
        std::size_t start = temp.find("\"");
        std::size_t end = temp.rfind("\"");

        std::string result;

        if (start != std::string::npos && end != std::string::npos)
        {
            result = temp.substr(start + 1, end);
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
        if (context.top().inValue)
        {
            value(context.top().name, getTempString());
            context.top().inValue = false;
        }
    }

    std::istream *data;
    std::size_t dataRead;

    bool        aborted;
    bool        inQuotes;

    std::string temp;

public:
    JSONParser(istream *_data):
        data(_data),
        dataRead(0),
        aborted(false),
        inQuotes(false)
    {

    }

    bool parse()
    {
        context.push(Context("<root>"));

        try
        {
            std::string c;
            char highChar = 0;

            const std::size_t bufferSize = 2048;
            char buffer[bufferSize];

            std::size_t read;

            while(!aborted)
            {
                data->read(buffer, bufferSize);
                read = data->gcount();

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
                                context.top().inValue = false;
                                onStartElement(context.top().name);

                                context.push(Context(context.top().name));
                                temp.clear();

                                break;

                            case '}':
                                checkValue();

                                if(context.top().inArray)
                                {
                                    onEndElement(context.top().name);
                                }
                                else
                                {
                                    onEndElement(context.top().name);
                                }

                                context.pop();
                                break;

                            case '[':
                                context.top().inValue = false;
                                context.top().inArray = true;
                                temp.clear();
                                onStartArray(context.top().name);
                                break;

                            case ']':
                                context.top().inArray = false;
                                onEndArray(context.top().name);
                                break;

                            case ':':
                                context.top().name = getTempString();
                                context.top().inValue = true;
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

    void onStartElement(std::string name)
    {

    }

    void onEndElement(std::string name)
    {

    }

    void onStartArray(std::string name)
    {

    }

    void onEndArray(std::string name)
    {

    }

    void value(std::string name, std::string value)
    {

    }
};