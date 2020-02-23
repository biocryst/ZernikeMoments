// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "loggers.h"

namespace io
{
    namespace xml
    {
        class XMLMerger
        {
            using logger_t = logging::logger_t;
            using severity_t = logging::severity_t;

        public:

            // node_name is name of node to select from temporary file
            XMLMerger(const std::string& path_to_res, const std::string& node_name);

            XMLMerger(const XMLMerger& other) = delete;

            ~XMLMerger();

            // Merge all temporary files
            bool merge_files(const std::vector < boost::filesystem::path >& xml_paths);

        private:
            bool merge_file(const boost::filesystem::path& path);

            xmlTextReaderPtr reader = nullptr;

            xmlTextWriterPtr writer = nullptr;

            std::string node_name;

            int rc{};
        };
    }
}