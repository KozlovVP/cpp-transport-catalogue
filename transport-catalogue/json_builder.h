#pragma once

#include <optional>
#include <vector>
#include <string>

#include "json.h"

namespace json {


class Builder {
public:
    Builder() {}

    Builder& Key(std::string key);
    Builder& Value(Node value);
    Builder& StartArray();
    Builder& StartDict();
    Builder& EndArray();
    Builder& EndDict();
    Node Build();
private:
    Builder& AddNode(Node value, bool stack_element);

    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;
};

class DictItemContext;
class DictValueContext;
class ArrayContext;
class ValueContext;

class BuilderContext {
public:
    BuilderContext(Builder& builder) : builder_(builder) {}
    DictValueContext Key(std::string key);
    ValueContext Value(Node value);
    ArrayContext StartArray();
    DictItemContext StartDict();
    BuilderContext EndArray();
    BuilderContext EndDict();
    Node Build();
protected:
    Builder& builder_;
};

class DictItemContext : public BuilderContext {
public:
    DictItemContext(Builder& builder) : BuilderContext(builder) {}
    ValueContext Value(Node node) = delete;
    BuilderContext StartArray() = delete;
    DictItemContext StartDict() = delete;
    BuilderContext EndArray() = delete;
    Node Build() = delete;
};

class DictValueContext : public BuilderContext {
public:
    DictValueContext(Builder& builder) : BuilderContext(builder) {}
    DictValueContext Key(std::string key) = delete;
    BuilderContext EndArray() = delete;
    BuilderContext EndDict() = delete;
    Node Build() = delete;
    DictItemContext Value(Node value);
};

class ValueContext : public BuilderContext {
public:
    ValueContext(Builder& builder) : BuilderContext(builder) {}
    DictValueContext& Key(std::string key) = delete;
};


class ArrayContext : public BuilderContext {
public:
    ArrayContext(Builder& builder) : BuilderContext(builder) {}
    DictValueContext Key(std::string key) = delete;
    BuilderContext EndDict() = delete;
    Node Build() = delete;
    ArrayContext Value(Node value);
};

} // namespace json
