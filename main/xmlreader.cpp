#include "xmlreader.h"

io::xml::XMLMerger::XMLMerger(const std::string& path_to_res, const std::string& node_name) : node_name(node_name)
{
    writer = xmlNewTextWriterFilename(path_to_res.c_str(), 0);

    if (writer == nullptr)
    {
        throw std::runtime_error("Cannot open xml");
    }
}

io::xml::XMLMerger::~XMLMerger()
{
    xmlFreeTextWriter(writer);
}

bool io::xml::XMLMerger::merge_files(const std::vector<boost::filesystem::path>& xml_paths)
{
    logger_t& logger = logging::logger_io::get();

    BOOST_LOG_SEV(logger, severity_t::info) << "Size: " << xml_paths.size() << std::endl;

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
        BOOST_LOG_SEV(logger, severity_t::error) << "Cannot write end of element" << std::endl;
        return false;
    }

    return true;
}

bool io::xml::XMLMerger::merge_file(const boost::filesystem::path& path)
{
    logger_t& logger = logging::logger_io::get();

    xmlTextReaderPtr reader{};

    BOOST_LOG_SEV(logger, severity_t::trace) << "Try to open: " << path << std::endl;

    reader = xmlReaderForFile(path.string().c_str(), NULL, XML_PARSE_RECOVER);

    if (reader != nullptr)
    {
        rc = xmlTextReaderRead(reader);

        while (rc == 1)
        {
            const xmlChar* name = xmlTextReaderConstName(reader);

            if (name != nullptr)
            {
                if (xmlStrEqual(name, BAD_CAST node_name.c_str()))
                {
                    xmlChar* content = xmlTextReaderReadOuterXml(reader);

                    if (content != nullptr)
                    {
                        xmlTextWriterWriteRaw(writer, content);
                    }

                    xmlFree(content);
                    rc = xmlTextReaderNext(reader);
                }
                else
                {
                    rc = xmlTextReaderRead(reader);
                }
            }
            else
            {
                rc = xmlTextReaderRead(reader);
            }
        }

        xmlFreeTextReader(reader);

        if (rc < 0)
        {
            BOOST_LOG_SEV(logger, severity_t::warning) << "Failed to parse" << std::endl;
            return false;
        }
    }
    else
    {
        BOOST_LOG_SEV(logger, severity_t::warning) << "Unable to open file" << std::endl;
        return false;
    }

    return true;
}