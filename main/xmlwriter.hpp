// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "loggers.h"

namespace io
{
    namespace xml
    {
        template <typename DescriptorType>
        class XMLWriter
        {
        public:

            XMLWriter(const boost::filesystem::path& path_to_xml, const boost::filesystem::path& root_voxel_dir) : root_dir(root_voxel_dir)
            {
                logger = &logging::logger_io::get();

                if (root_dir.empty())
                {
                    throw std::runtime_error(u8"root_dir is empty");
                }

                writer = xmlNewTextWriterFilename(path_to_xml.string().c_str(), 0);

                if (writer == nullptr)
                {
                    throw std::runtime_error(u8"Error creating the xml writer");
                }

                rc = xmlTextWriterStartDocument(writer, nullptr, u8"UTF-8", nullptr);

                if (rc < 0)
                {
                    throw std::runtime_error(u8"Error when trying to write start of document");
                }

                rc = xmlTextWriterStartElement(writer, BAD_CAST u8"Voxels");

                if (rc < 0)
                {
                    throw std::runtime_error(u8"Error when trying to write start of element");
                }

                rc = xmlTextWriterWriteAttribute(writer, BAD_CAST u8"RootDir", BAD_CAST root_dir.string().c_str());

                if (rc < 0)
                {
                    throw std::runtime_error(u8"Error when trying to write start of element");
                }
            }

            XMLWriter(const XMLWriter&) = delete;

            ~XMLWriter()
            {
                rc = xmlTextWriterEndDocument(writer);

                if (rc < 0)
                {
                    BOOST_LOG_SEV(*logger, logging::severity_t::error) << u8"Cannot write end of document" << std::endl;
                }

                xmlFreeTextWriter(writer);
                logger = nullptr;
            }

            bool write_descriptor(const boost::filesystem::path& path_to_voxel, size_t voxel_res, const std::vector<DescriptorType>& desc)
            {
                static_assert(std::is_floating_point<DescriptorType>::value, "Expected floating point type: float, double or long double");

                rc = xmlTextWriterStartElement(writer, BAD_CAST u8"Voxel");

                if (rc < 0)
                {
                    return  false;
                }

                boost::filesystem::path rel_path{ boost::filesystem::relative(path_to_voxel, root_dir) };

                rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST u8"Path", "%s", rel_path.string().c_str());

                if (rc < 0)
                {
                    return false;
                }

                rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST u8"GridResolution", "%zu", voxel_res);

                if (rc < 0)
                {
                    return false;
                }

                rc = xmlTextWriterStartElement(writer, BAD_CAST u8"Descriptor");

                if (rc < 0)
                {
                    return  false;
                }

                rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST u8"Size", "%zu", desc.size());

                if (rc < 0)
                {
                    return  false;
                }

                for (size_t i = 0; i < desc.size(); i++)
                {
                    std::string attr_name{ u8"Value" };

                    attr_name += std::to_string(i + 1);

                    rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST attr_name.c_str(), double_formatter(), desc[i]);

                    if (rc < 0)
                    {
                        return  false;
                    }
                }

                rc = xmlTextWriterEndElement(writer);

                if (rc < 0)
                {
                    return  false;
                }

                rc = xmlTextWriterEndElement(writer);

                if (rc < 0)
                {
                    return  false;
                }

                return true;
            }
        private:

            // Return formatter for printf, when type is long double
            // bool value unused
            template<typename T_ = DescriptorType, typename = std::enable_if_t<std::is_same<long double, T_>::value>>
            constexpr const char* double_formatter(bool = false)
            {
                return  "%Lg";
            }

            // Return formatter for printf
            template<typename T_ = DescriptorType, typename = std::enable_if_t<!std::is_same<long double, T_>::value>>
            constexpr const char* double_formatter()
            {
                return "%g";
            }

            int rc{};

            logging::logger_t* logger = nullptr;

            xmlTextWriterPtr writer = nullptr;

            boost::filesystem::path root_dir;
        };
    }
}