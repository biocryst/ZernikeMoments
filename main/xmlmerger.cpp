// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "xmlmerger.h"

io::xml::XMLMerger::XMLMerger(const std::string& path_to_res, const std::string& node_name) : node_name(node_name)
{
    writer = xmlNewTextWriterFilename(path_to_res.c_str(), 0);

    if (writer == nullptr)
    {
        throw std::runtime_error(u8"Cannot open xml");
    }

    if (node_name.empty())
    {
        xmlFreeTextWriter(writer);
        throw std::invalid_argument(u8"node_name is empty");
    }
}

io::xml::XMLMerger::~XMLMerger()
{
    xmlFreeTextWriter(writer);
}

bool io::xml::XMLMerger::merge_files(const std::vector<boost::filesystem::path>& xml_paths)
{
    logger_t& logger = logging::logger_io::get();

    BOOST_LOG_SEV(logger, severity_t::info) << u8"Number of files to merge: " << xml_paths.size() << std::endl;

    rc = xmlTextWriterStartDocument(writer, NULL, u8"UTF-8", NULL);

    if (rc < 0)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << "Cannot write start of document" << std::endl;
        return false;
    }

    rc = xmlTextWriterStartElement(writer, BAD_CAST u8"Voxels");

    if (rc < 0)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << "Cannot write root element" << std::endl;
        return false;
    }

    for (const auto& path : xml_paths)
    {
        BOOST_LOG_SEV(logger, severity_t::trace) << "Merge: " << path << std::endl;

        if (!merge_file(path))
        {
            BOOST_LOG_SEV(logger, severity_t::warning) << "Cannot merge " << path << std::endl;
        }
    }

    rc = xmlTextWriterEndDocument(writer);

    if (rc < 0)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << "Cannot write end of document" << std::endl;
        return false;
    }

    return true;
}

bool io::xml::XMLMerger::merge_file(const boost::filesystem::path& path)
{
    logger_t& logger = logging::logger_io::get();

    BOOST_LOG_SEV(logger, severity_t::trace) << u8"Try to open: " << path << std::endl;

    auto xml_deleter = [](xmlTextReader* reader)
    {
        xmlFreeTextReader(reader);
    };

    auto xml_creator = [](const boost::filesystem::path& path)
    {
        return static_cast<xmlTextReader*>(xmlReaderForFile(path.string().c_str(), u8"UTF-8", 0));
    };

    std::unique_ptr <xmlTextReader, decltype(xml_deleter)> reader = std::unique_ptr<xmlTextReader, decltype(xml_deleter)>(xml_creator(path), xml_deleter);

    if (reader.get() != nullptr)
    {
        rc = xmlTextReaderRead(reader.get());

        while (rc == 1)
        {
            const xmlChar* name = xmlTextReaderConstName(reader.get());

            if (name != nullptr)
            {
                if (xmlStrEqual(name, BAD_CAST node_name.c_str()))
                {
                    xmlChar* content = xmlTextReaderReadOuterXml(reader.get());

                    if (content != nullptr)
                    {
                        xmlTextWriterWriteRaw(writer, content);
                    }

                    xmlFree(content);

                    rc = xmlTextReaderNext(reader.get());
                    continue;
                }
            }

            rc = xmlTextReaderRead(reader.get());
        }

        if (rc < 0)
        {
            BOOST_LOG_SEV(logger, severity_t::warning) << u8"Failed to parse XML" << std::endl;
            return false;
        }
    }
    else
    {
        BOOST_LOG_SEV(logger, severity_t::warning) << u8"Unable to open " << path << std::endl;
        return false;
    }

    return true;
}