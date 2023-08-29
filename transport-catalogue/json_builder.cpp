#include <stdexcept>

#include "json_builder.h"

namespace json {

using namespace std::literals;

DictValueContext BuilderContext::Key(std::string key) {
    return DictValueContext(builder_.Key(std::move(key)));
}

ValueContext BuilderContext::Value(Node value) {
    return ValueContext(builder_.Value(std::move(value)));
}

ArrayContext BuilderContext::StartArray() {
    return ArrayContext(builder_.StartArray());
}

DictItemContext BuilderContext::StartDict() {
    return DictItemContext(builder_.StartDict());
}

BuilderContext BuilderContext::EndArray() {
    return BuilderContext(builder_.EndArray());
}

BuilderContext BuilderContext::EndDict() {
    return BuilderContext(builder_.EndDict());
}

Node BuilderContext::Build() {
    return builder_.Build();
}

DictItemContext DictValueContext::Value(Node value) {
    return DictItemContext(builder_.Value(value));
}

ArrayContext ArrayContext::Value(Node value) {
    return ArrayContext(builder_.Value(value));
}

Builder& Builder::Key(std::string key) {
    Node* node = (nodes_stack_.empty()) ? &root_ : nodes_stack_.back();
    if (!node->IsDict()) {
        throw std::logic_error("Not dictionary"s);
    }

    if (key_.has_value()) {
        throw std::logic_error("Double key"s);
    }

    key_ = std::move(key);
    return *this;
}

Builder& Builder::Value(Node value) {
    return AddNode(std::move(value), false);
}

Builder& Builder::StartArray() {
    return AddNode(std::move(Array{}), true);
}

Builder& Builder::StartDict() {
    return AddNode(std::move(Dict{}), true);
}

Builder& Builder::EndArray() {
    Node* node = (nodes_stack_.empty()) ? &root_ : nodes_stack_.back();
    if (!node->IsArray()) {
        throw std::logic_error("Not Array"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndDict() {
    Node* node = (nodes_stack_.empty()) ? &root_ : nodes_stack_.back();
    if (!node->IsDict()) {
        throw std::logic_error("Not Dictionary"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty() > 0) {
        throw std::logic_error("Have open Array"s);
    }

    if(root_== nullptr) {
        throw std::logic_error("No data"s);
    }

    return root_;
}

Builder& Builder::AddNode(Node value, bool stack_element) {
    Node* node = (nodes_stack_.empty()) ? &root_ : nodes_stack_.back();
    if (node->IsArray()) {
        std::get<Array>(node->GetValue()).emplace_back(std::move(value));
        if (stack_element) {
            nodes_stack_.push_back(&std::get<Array>(node->GetValue()).back());
        }
        return *this;
    }

    if (node->IsDict()) {
        if (!key_.has_value()) {
            throw std::logic_error("Key must be here"s);
        }
        std::get<Dict>(node->GetValue())[key_.value()] = std::move(value);
        if (stack_element) {
            nodes_stack_.push_back(&std::get<Dict>(node->GetValue())[key_.value()]);
        }
        key_.reset();
        return *this;
    }

    if (!node->IsNullptr()) {
        throw std::logic_error("Double value"s);
    }

    *node = std::move(value);
    if (stack_element) {
        nodes_stack_.push_back(node);
    }
    return *this;
}

} // namespace json
