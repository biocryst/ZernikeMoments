#pragma once

#include "stdafx.h"

template<typename DataType>
class Node
{
public:

    Node() = delete;

    Node(const boost::filesystem::path & path) : Node(path, nullptr)
    {
    }

    Node(const boost::filesystem::path & path, const DataType & data) : _entity_name(path), _data(data)
    {
    }

    Node(const Node & node) = default;

    ~Node() = default;

    bool operator==(const Node & other) const
    {
        return _entity_name == other._entity_name;
    }

    bool operator!=(const Node & other) const
    {
        return _entity_name != other._entity_name;
    }

    bool contain_

private:
    std::set<Node> _childs;
    boost::filesystem::path _entity_name;
    std::unique_ptr < DataType> _data;
};

template<typename NodeType>
class PathTree
{
public:
    PathTree() = default;

    PathTree(const NodeType & node) : _root(node)
    {
    }

    PathTree(const & PathTree tree) = delete;

    ~PathTree() = default;

    bool path_exist(const boost::filesystem::path & path)
    {
    }

private:
    std::unique_ptr<NodeType> _root;
};