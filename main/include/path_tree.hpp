// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "db.h"

namespace tree
{
    template<typename HashDataT>
    class Node
    {
    public:

        Node(const boost::filesystem::path & path_part, const HashDataT & data) : _path_part(path_part), _data(std::make_unique<HashDataT>(data))
        {
        }

        Node(const boost::filesystem::path & path_part) : _path_part(path_part)
        {
        }

        ~Node() = default;

        Node(const Node & node) = default;

        bool operator==(const Node & other) const
        {
            return _path_part == other._path_part;
        }

        bool operator==(const boost::filesystem::path & part) const
        {
            return _path_part == part;
        }

        bool operator!=(const boost::filesystem::path & part) const
        {
            return _path_part != part;
        }

        bool operator!=(const Node & other) const
        {
            return _path_part != other._path_part;
        }

        auto find_or_insert(const std::shared_ptr<Node> node)
        {
            return _childs.insert(node);
        }

        bool is_leaf() const
        {
            return _childs.empty();
        }

        boost::filesystem::path path_part() const
        {
            return _path_part;
        }

        auto cbegin() const
        {
            return _childs.cbegin();
        }

        auto cend() const
        {
            return _childs.cend();
        }

        HashDataT data() const
        {
            if (_data == nullptr)
            {
                return nullptr;
            }
            else
            {
                return *_data;
            }
        }

    private:

        struct NodeCmpLess
        {
            bool operator() (const std::shared_ptr<Node> & node1, const std::shared_ptr<Node> & node2) const
            {
                if (node1 == nullptr || node2 == nullptr)
                {
                    return false;
                }

                return node1->_path_part < node2->_path_part;
            }
        };

        std::set<std::shared_ptr<Node>, NodeCmpLess> _childs;
        boost::filesystem::path _path_part;
        std::unique_ptr<HashDataT> _data;
    };

    template<typename NodeType>
    class PathTree
    {
    public:

        PathTree() = delete;

        ~PathTree() = default;

        PathTree(const boost::filesystem::path & root_path)
        {
            _root = std::make_shared<NodeType>(root_path);
        }

        PathTree(const PathTree & other) = default;

        static PathTree build_tree_from_db(const boost::filesystem::path & root_path, sqlite::database & db)
        {
            PathTree<NodeType> tree(root_path);

            std::stringstream select_query{};
            select_query << u8"SELECT "
                << db::DbSchema::path_column() << ','
                << db::DbSchema::file_hash_column()
                << u8" FROM " << db::DbSchema::table_name();

            std::shared_ptr<NodeType> node;

            db << select_query.str()
                >> [&tree, &root_path, &node](const std::string & path, const std::string & file_hash) -> void
            {
                tree.add_path(root_path / path, file_hash, node);
            };

            return tree;
        }

        // Path must have root as parent path.
        // Return true if path does not exist
        // Return false if path exits in the tree
        bool add_path(const boost::filesystem::path & path, const std::string & hash_value, std::shared_ptr<NodeType> & inserted_node)
        {
            if (_root == nullptr)
            {
                throw std::runtime_error("Root of tree is null");
            }

            if (!path.is_absolute())
            {
                throw std::invalid_argument("An input path must be absolute.");
            }

            auto relative_path{ boost::filesystem::relative(path, _root->path_part()) };

            std::shared_ptr<NodeType> current_node{ _root };

            boost::filesystem::path separator;
            separator += boost::filesystem::path::preferred_separator;

            bool is_new_node{ false };

            for (const auto & part : relative_path)
            {
                if (part == separator)
                {
                    continue;
                }

                // It is leaf
                if (part == path.filename())
                {
                    auto node = current_node->find_or_insert(std::make_shared<NodeType>(part, hash_value));
                    current_node = *node.first;
                    is_new_node = node.second;
                }
                else
                {
                    auto node = current_node->find_or_insert(std::make_shared<NodeType>(part));
                    current_node = *node.first;
                    is_new_node = node.second;
                }
            }

            inserted_node = current_node;

            return is_new_node;
        }

        friend std::ostream & operator<<(std::ostream & stream, const PathTree & tree)
        {
            std::stack<std::tuple<std::shared_ptr<NodeType>, size_t>> nodes;
            nodes.push(std::make_tuple(tree.root(), 0));

            while (!nodes.empty())
            {
                auto current_node_indent = nodes.top();
                nodes.pop();

                for (size_t i = 0; i < std::get<1>(current_node_indent); i++)
                {
                    stream << ' ';
                }

                auto current_node = std::get<0>(current_node_indent);

                stream << current_node->path_part() << std::endl;

                for (auto child_it = current_node->cbegin(); child_it != current_node->cend(); ++child_it)
                {
                    nodes.push(std::make_tuple(*child_it, 1 + std::get<1>(current_node_indent)));
                }
            }

            return stream;
        }

        std::shared_ptr<NodeType> root() const
        {
            return _root;
        }

    private:
        std::shared_ptr<NodeType> _root;
    };
}