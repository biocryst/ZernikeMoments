// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "db.h"

namespace sqldata
{
    template<typename DescriptorType>
    struct Row
    {
        std::string generic_path;
        std::string file_hash;
        std::vector <DescriptorType> descriptor;
        int max_order;

        Row(const std::string & generic_path, const std::string & hash, const std::vector <DescriptorType> & descriptor, int max_order) : generic_path(generic_path), file_hash(hash), descriptor(descriptor), max_order(max_order)
        {
        }
    };

    template<typename DescriptorType>
    sqlite::database & operator<<(sqlite::database & db, const Row<DescriptorType> & row)
    {
        std::stringstream insert_query;

        using namespace db;

        insert_query << u8"INSERT INTO " << DbSchema::table_name() << '('
            << DbSchema::path_column() << ','
            << DbSchema::file_hash_column() << ','
            << DbSchema::desc_length_column() << ','
            << DbSchema::desc_value_size_bytes_column() << ','
            << DbSchema::descriptor_column() << ','
            << DbSchema::max_order_column() << u8") VALUES (?, ?, ?, ?, ?, ?)";
        db << insert_query.str()
            << row.generic_path
            << row.file_hash
            << row.descriptor.size()
            << sizeof(DescriptorType)
            << row.descriptor
            << row.max_order;

        return db;
    }

    template<typename DescriptorType>
    sqlite::database_binder & operator<<(sqlite::database_binder & db_binder, const Row<DescriptorType> & row)
    {
        db_binder << row.generic_path
            << row.file_hash
            << row.descriptor.size()
            << sizeof(DescriptorType)
            << row.descriptor
            << row.max_order;
        db_binder++;

        return db_binder;
    }

    template<typename TData>
    class CollectionRows
    {
        using RowT = Row<TData>;
    public:
        CollectionRows() = default;

        ~CollectionRows() = default;

        CollectionRows(const CollectionRows & collection) = delete;

        void add_row(const RowT & row)
        {
            _rows.push_back(row);
        }

        template<typename ... Args>
        void emplace_row(Args && ...args)
        {
            _rows.emplace_back(std::forward<Args>(args)...);
        }

        friend sqlite::database & operator<<(sqlite::database & db, const CollectionRows <TData > & row_collection)
        {
            std::stringstream insert_query;

            using namespace db;

            insert_query << u8"INSERT INTO " << DbSchema::table_name() << '('
                << DbSchema::path_column() << ','
                << DbSchema::file_hash_column() << ','
                << DbSchema::desc_length_column() << ','
                << DbSchema::desc_value_size_bytes_column() << ','
                << DbSchema::descriptor_column() << ','
                << DbSchema::max_order_column() << u8") VALUES (?, ?, ?, ?, ?, ?)";

            auto query = db << insert_query.str();

            for (const auto & row : row_collection._rows)
            {
                query << row;
            }

            return db;
        }

        void clear()
        {
            _rows.clear();
        }

        size_t size() const
        {
            return _rows.size();
        }

        bool empty() const
        {
            return _rows.empty();
        }

    private:
        std::vector < RowT> _rows;
    };
}