#pragma once

#include "stdafx.h"

namespace db
{
    class DbSchema
    {
    public:
        static constexpr const char * id_column()
        {
            return u8"id";
        }

        static constexpr const char * path_column()
        {
            return u8"path";
        }

        static constexpr const char * file_hash_column()
        {
            return u8"file_hash_sha256";
        }

        static constexpr const char * max_order_column()
        {
            return u8"max_order";
        }

        static constexpr const char * desc_length_column()
        {
            return u8"desc_length";
        }

        static constexpr const char * desc_value_size_bytes_column()
        {
            return u8"desc_value_size_bytes";
        }

        static constexpr const char * descriptor_column()
        {
            return u8"descriptor";
        }

        static constexpr const char * table_name()
        {
            return u8"zernike_descriptors";
        }

        static std::string create_table_ddl()
        {
            std::stringstream query;
            query << u8"CREATE TABLE IF NOT EXISTS "
                << table_name()
                << u8" (" << id_column() << u8" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                << path_column() << u8" TEXT NOT NULL CHECK(length(" << path_column() << u8") > 0),"
                << file_hash_column() << u8" TEXT NOT NULL CHECK(length(" << file_hash_column() << u8") > 0),"
                << max_order_column() << u8" INTEGER NOT NULL CHECK(" << max_order_column() << u8" > 0), "
                << desc_length_column() << u8" INTEGER NOT NULL CHECK(" << desc_length_column() << u8" > 0),"
                << desc_value_size_bytes_column() << u8" INTEGER NOT NULL CHECK(" << desc_value_size_bytes_column() << u8" > 0),"
                << descriptor_column() << u8" BLOB"
                << ')';

            return query.str();
        }

        static void init_db(sqlite::database & db)
        {
            std::string ddl_query{ create_table_ddl() };
            db << ddl_query;

            {
                std::stringstream file_hash_index_query;
                file_hash_index_query << u8"CREATE INDEX IF NOT EXISTS hash_index ON " << table_name() << u8" (" << file_hash_column() << ')';

                db << file_hash_index_query.str();
            }

            {
                std::stringstream path_index_query;
                path_index_query << u8"CREATE INDEX IF NOT EXISTS path_index ON " << table_name() << u8" (" << path_column() << ')';
                db << path_index_query.str();
            }
        }
    };
}
