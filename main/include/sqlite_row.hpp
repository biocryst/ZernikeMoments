#pragma once

#include "stdafx.h"

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
        db << u8"INSERT INTO zernike_descriptors (path, file_hash, desc_length, desc_value_size_bytes, descriptor, max_order) VALUES(?, ?, ?, ?, ?, ?)"
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
        using Row = Row<TData>;
    public:
        CollectionRows() = default;

        ~CollectionRows() = default;

        CollectionRows(const CollectionRows & collection) = delete;

        void add_row(const Row & row)
        {
            _rows.push_back(row);
        }

        template<typename ... Args>
        void emplace_row(Args && ...args)
        {
            _rows.emplace_back(std::forward<Args>(args)...);
        }

        template<typename TData = TData>
        friend sqlite::database & operator<<(sqlite::database & db, const CollectionRows <TData > & row_collection)
        {
            auto query = db << u8"INSERT INTO zernike_descriptors (path, file_hash, desc_length, desc_value_size_bytes, descriptor, max_order) VALUES(?, ?, ?, ?, ?, ?)";

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
        std::vector < Row> _rows;
    };
}