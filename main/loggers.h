#pragma once

#include "stdafx.h"

namespace logging
{
	using severity_t = boost::log::trivial::severity_level;
	using logger_t = boost::log::sources::severity_channel_logger_mt<severity_t>;

	BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(logger_io, logging::logger_t, (boost::log::keywords::channel = "io"))
		BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(logger_main, logging::logger_t, (boost::log::keywords::channel = "main"))
		BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(logger_z3d, logging::logger_t, (boost::log::keywords::channel = "z3d"))
}