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

            XMLMerger(const std::string& path_to_res, const std::string& node_name);

            XMLMerger(const XMLMerger& other) = delete;

            ~XMLMerger();

            // Merge all temporary files
            bool merge_files(const std::vector < boost::filesystem::path >& xml_paths);

            bool merge_file(const boost::filesystem::path& path);

        private:
            xmlTextReaderPtr reader = nullptr;

            xmlTextWriterPtr writer = nullptr;

            std::string node_name;

            int rc{};
        };
    }
}