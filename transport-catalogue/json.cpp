#include "json.h"

#include <unordered_map>

using namespace std;

namespace json {

namespace {

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

Node LoadString(istream& input) {
    string str;
    for (char ch; input.get(ch) && ch != '\"' ;) {
        if (ch == '\\') {
            if (!input.get(ch)) {
                throw ParsingError("Failed to load String");
            }
            if (ch == 'n') {
                str += '\n';
                continue;
            }
            if (ch == 'r') {
                str += '\r';
                continue;
            }
            if (ch == '\"') {
                str += '\"';
                continue;
            }
            if (ch == 't') {
                str += '\t';
                continue;
            }
            if (ch == '\\') {
                str += '\\';
                continue;
            }
        } else {
            str += ch;
        }
    }

    if (input.eof()) {
        throw ParsingError("Failed to load String");
    }

    return Node(move(str));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    do {
        input >> c;
    } while ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'));

    if (c == ']' || c == '}')
    {
        throw ParsingError("Failed to parse }]");
    }

    if (c == '[') {
        if (input.peek() == std::istream::traits_type::eof()) {
            throw ParsingError("Failed to load Array");
        }
        return LoadArray(input);
    } else if (c == '{') {
        if (input.peek() == std::istream::traits_type::eof()) {
            throw ParsingError("Failed to load Dict");
        }
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if ((c == 't') || (c == 'f') || (c == 'n')) {
        input.putback(c);
        string str;
        //getline(input, str, ',');

        for (;input >> c && (c != ' ') && (c != '\t') && (c != '\r') && (c != '\n') && (c != ',') && (c != ']') && (c != '}');) {
            str += c;
        }
        input.putback(c);

        if (str == "true") {
            return Node(true);
        }
        if (str == "false") {
            return Node(false);
        }
        if (str == "null") {
            return Node();
        }
        throw ParsingError("Failed to load Bool");
    } else {
        input.putback(c);
        Number number = LoadNumber(input);
        return (holds_alternative<int>(number) ?
                Node(std::get<int>(number)) : Node(std::get<double>(number)));
    }
}

}  // namespace

bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(data_);
}

bool Node::IsInt() const {
    return holds_alternative<int>(data_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(data_);
}

bool Node::IsDouble() const {
    return (holds_alternative<double>(data_) || IsInt());
}

bool Node::IsPureDouble() const {
    return (holds_alternative<double>(data_) && !IsInt());
}

bool Node::IsString() const {
    return holds_alternative<std::string>(data_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(data_);
}

size_t Node::GetIndex() const {
    return data_.index();
}

const NodeData& Node::GetData() const {
    return data_;
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Failed to AsArray");
    }
    return std::get<Array>(data_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Failed to AsMap");
    }
    return std::get<Dict>(data_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Failed to AsBool");
    }
    return std::get<bool>(data_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Failed to AsInt");
    }
    return std::get<int>(data_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Failed to AsDouble");
    }
    return IsPureDouble() ? std::get<double>(data_) : std::get<int>(data_);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Failed to AsString");
    }
    return std::get<std::string>(data_);
}

bool Node::operator==(const json::Node& other) const {
    return (this->GetIndex() == other.GetIndex() && this->GetData() == other.GetData());
}

bool Node::operator!=(const json::Node& other) const {
    return !(*this == other);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return this->root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// helper constant for the visitor #3
template<class> inline constexpr bool always_false_v = false;

void Print(const Document& doc, std::ostream& output) {
    //std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    // Реализуйте функцию самостоятельно
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            output << "null";
            return;
        } else if constexpr (std::is_same_v<T, double>) {
            output << doc.GetRoot().AsDouble();
            return;
        } else if constexpr (std::is_same_v<T, int>) {
            output << doc.GetRoot().AsInt();
            return;
        } else if constexpr (std::is_same_v<T, bool>) {
            output << std::boolalpha
                   << doc.GetRoot().AsBool()
                   << std::noboolalpha;
            return;
        } else if constexpr (std::is_same_v<T, std::string>) {
            string str;
            for (const auto ch : doc.GetRoot().AsString()) {
                if (ch == '\n') {
                    str += "\\n"s;
                    continue;
                }
                if (ch == '\r') {
                    str += "\\r"s;
                    continue;
                }
                if (ch == '\"') {
                    str += "\\\""s;
                    continue;
                }
                if (ch == '\t') {
                    //str += "\\t"s;
                    str += "\t"s;
                    continue;
                }
                if (ch == '\\') {
                    str += "\\\\"s;
                    continue;
                }

                str += ch;
            }
            output << "\""s << str << "\""s;
            return;
        } else if constexpr (std::is_same_v<T, Array>) {
            const auto& arr = doc.GetRoot().AsArray();
            output << "["sv;
            bool first = true;
            for (const auto& node : arr) {
                if (!first) {
                    output << ","sv;
                }
                first = false;
                Document doc_elt(node);
                Print(doc_elt, output);
            }
            output << "]"sv;
            return;
        } else if constexpr (std::is_same_v<T, Dict>) {
            const auto& dicts = doc.GetRoot().AsMap();
            output << "{"sv;
            bool first = true;
            for (const auto& dict : dicts) {
                if (!first) {
                    output << ","sv;
                }
                first = false;
                output << "\""sv << dict.first << "\": "sv;
                Document doc_elt(dict.second);
                Print(doc_elt, output);
            }
            output << "}"sv;
            return;
        } else {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, doc.GetRoot().GetData());
}

}  // namespace json

