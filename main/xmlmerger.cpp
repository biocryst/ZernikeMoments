// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "xmlmerger.h"

io::xml::XMLMerger::XMLMerger(const std::string& path_to_res, const std::string& node_name, const std::string& root_node_name) : node_name(node_name), root_node_name(root_node_name)
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

    if (root_node_name.empty())
    {
        xmlFreeTextWriter(writer);
        throw std::invalid_argument(u8"root_node_name is empty");
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

    rc = xmlTextWriterStartElement(writer, BAD_CAST root_node_name.c_str());

    if (rc < 0)
    {
        BOOST_LOG_SEV(logger, severity_t::warning) << u8"Failed to write start of element." << std::endl;
        return false;
    }

    bool is_first = true;

    for (const auto& path : xml_paths)
    {
        BOOST_LOG_SEV(logger, severity_t::trace) << "Merge: " << path << std::endl;

        if (!merge_file(path, is_first))
        {
            BOOST_LOG_SEV(logger, severity_t::warning) << "Cannot merge " << path << std::endl;
        }
        else
        {
            is_first = false;
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

bool io::xml::XMLMerger::merge_file(const boost::filesystem::path& path, bool is_first)
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

    const xmlChar* attr_name = BAD_CAST u8"RootDir";

    bool is_root_found = !is_first;

    if (reader.get() != nullptr)
    {
        rc = xmlTextReaderRead(reader.get());

        while (rc == 1)
        {
            const xmlChar* name = xmlTextReaderConstName(reader.get());

            if (name != nullptr)
            {
                if (is_first && xmlStrEqual(name, BAD_CAST root_node_name.c_str()))
                {
                    xmlNodePtr node = xmlTextReaderCurrentNode(reader.get());

                    if (node != nullptr)
                    {
                        xmlAttr* attr = node->properties;

                        while (attr != nullptr)
                        {
                            if (attr->name)
                            {
                                if (xmlStrEqual(attr->name, attr_name))
                                {
                                    xmlChar* value = xmlNodeListGetString(attr->doc, attr->children, 0);
                                    BOOST_LOG_SEV(logger, severity_t::debug) << "Read " << (const char*)value << std::endl;
                                    xmlTextWriterWriteAttribute(writer, attr_name, value);
                                    xmlFree(value);
                                }
                            }
                            attr = attr->next;
                        }
                    }

                    is_root_found = true;
                }
                else if (xmlStrEqual(name, BAD_CAST node_name.c_str()))
                {
                    if (!is_root_found)
                    {
                        BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot find root node. XML is incorrect." << std::endl;
                        return false;
                    }

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