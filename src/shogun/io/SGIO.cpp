/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Soeren Sonnenburg, Giovanni De Toni, Viktor Gal, Heiko Strathmann,
 *          Thoralf Klein, Evangelos Anagnostopoulos, Weijie Lin, Bjoern Esser,
 *          Saurabh Goyal
 */

#include <shogun/io/SGIO.h>
#include <shogun/lib/RefCount.h>
#include <shogun/lib/Time.h>
#include <shogun/lib/common.h>
#include <shogun/lib/memory.h>
#include <shogun/mathematics/Math.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <sstream>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <mutex>
#include <stdlib.h>

#ifdef _WIN32
#define R_OK 4
#endif

#define FBUFSIZE 4096

using namespace shogun;

class SGIO::RedirectSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
	RedirectSink()
	{
		stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
	}

	void redirect_stdout(std::shared_ptr<spdlog::sinks::sink> sink_)
	{
		set_sink_(stdout_sink, sink_);
	}

	void redirect_stderr(std::shared_ptr<spdlog::sinks::sink> sink_)
	{
		set_sink_(stderr_sink, sink_);
	}

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		if (msg.level == spdlog::level::err)
			stderr_sink->log(msg);
		else
			stdout_sink->log(msg);
	}

	void flush_() override
	{
		stdout_sink->flush();
		if (stdout_sink != stderr_sink)
			stderr_sink->flush();
	}

	void set_pattern_(const std::string& pattern) override
	{
		set_formatter_(
		    spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
	}

	void
	set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override
	{
		formatter_ = std::move(sink_formatter);
		stdout_sink->set_formatter(formatter_->clone());
		if (stdout_sink != stderr_sink)
			stderr_sink->set_formatter(formatter_->clone());
	}

private:
	void set_sink_(
	    std::shared_ptr<spdlog::sinks::sink>& old_sink,
	    const std::shared_ptr<spdlog::sinks::sink>& new_sink)
	{
		flush();
		std::lock_guard<std::mutex> lock(mutex_);
		if (new_sink)
			old_sink = new_sink;
		else
			old_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
	}

	std::shared_ptr<spdlog::sinks::sink> stdout_sink;
	std::shared_ptr<spdlog::sinks::sink> stderr_sink;
};

SGIO::SGIO()
    : show_progress(false), location_info(MSG_NONE),
      syntax_highlight(true)
{
	m_refcount = new RefCount();
	io_sink = std::make_shared<RedirectSink>();

	const int queue_size = 128;
	const int n_threads = 1;
	spdlog::init_thread_pool(queue_size, n_threads);

	io_logger = std::make_shared<spdlog::async_logger>(
	    "global", io_sink, spdlog::thread_pool(),
	    spdlog::async_overflow_policy::block);
	update_pattern();
}

SGIO::SGIO(const SGIO& orig)
    : show_progress(orig.get_show_progress()),
      location_info(orig.get_location_info()),
      syntax_highlight(orig.get_syntax_highlight()), io_sink(orig.io_sink),
      io_logger(orig.io_logger)
{
	m_refcount = new RefCount();
}

std::string SGIO::format(const char* fmt, ...) const
{
	char str[FBUFSIZE];

	va_list list;
	va_start(list, fmt);
	vsnprintf(str, sizeof(str), fmt, list);
	va_end(list);

	return std::string(str);
}

void SGIO::print(
    EMessageType prio, const spdlog::source_loc& loc,
    const std::string& msg) const
{
	io_logger->log(loc, static_cast<spdlog::level::level_enum>(prio), msg);
}

void SGIO::done()
{
	if (!show_progress)
		return;

	message(MSG_INFO, "done.\n");
}

char* SGIO::skip_spaces(char* str)
{
	int32_t i=0;

	if (str)
	{
		for (i=0; isspace(str[i]); i++);

		return &str[i];
	}
	else
		return str;
}

char* SGIO::skip_blanks(char* str)
{
	int32_t i=0;

	if (str)
	{
		for (i=0; isblank(str[i]); i++);

		return &str[i];
	}
	else
		return str;
}

EMessageType SGIO::get_loglevel() const
{
	return static_cast<EMessageType>(io_logger->level());
}

void SGIO::set_loglevel(EMessageType level)
{
	io_logger->set_level(static_cast<spdlog::level::level_enum>(level));
}

char* SGIO::c_string_of_substring(substring s)
{
	uint32_t len = s.end - s.start+1;
	char* ret = SG_CALLOC(char, len);
	sg_memcpy(ret,s.start,len-1);
	return ret;
}

void SGIO::print_substring(substring s)
{
	char* c_string = c_string_of_substring(s);
	SG_SPRINT("%s\n", c_string)
	SG_FREE(c_string);
}

float32_t SGIO::float_of_substring(substring s)
{
	char* endptr = s.end;
	float32_t f = strtof(s.start,&endptr);
	if (endptr == s.start && s.start != s.end)
		SG_SERROR("error: %s is not a float!\n", c_string_of_substring(s))

	return f;
}

float64_t SGIO::double_of_substring(substring s)
{
	char* endptr = s.end;
	float64_t f = strtod(s.start,&endptr);
	if (endptr == s.start && s.start != s.end)
		SG_SERROR("Error!:%s is not a double!\n", c_string_of_substring(s))

	return f;
}

int32_t SGIO::int_of_substring(substring s)
{
	char* c_string = c_string_of_substring(s);
	int32_t int_val = atoi(c_string);
	SG_FREE(c_string);

	return int_val;
}

uint32_t SGIO::ulong_of_substring(substring s)
{
	return strtoul(s.start,NULL,10);
}

uint32_t SGIO::ss_length(substring s)
{
	return (s.end - s.start);
}

SGIO::~SGIO()
{
	delete m_refcount;
}

int32_t SGIO::ref()
{
	return m_refcount->ref();
}

int32_t SGIO::ref_count() const
{
	return m_refcount->ref_count();
}

int32_t SGIO::unref()
{
	int32_t rc = m_refcount->unref();
	if (rc==0)
	{
		delete this;
		return 0;
	}

	return rc;
}

void SGIO::update_pattern()
{
	std::stringstream pattern_builder;
	pattern_builder << "[%D %T ";
	switch(location_info)
	{
	case MSG_LINE_AND_FILE:
		pattern_builder << "%@ ";
		break;
	case MSG_FUNCTION:
		pattern_builder << "%! ";
		break;
	default:
		break;
	}
	if (syntax_highlight)
		pattern_builder << "%^%l%$] ";
	else
		pattern_builder << "%l] ";

	pattern_builder << "%v";

	io_logger->set_pattern(pattern_builder.str());
}

void SGIO::redirect_stdout(std::shared_ptr<spdlog::sinks::sink> sink)
{
	io_sink->redirect_stdout(sink);
	update_pattern();
}

void SGIO::redirect_stderr(std::shared_ptr<spdlog::sinks::sink> sink)
{
	io_sink->redirect_stderr(sink);
	update_pattern();
}
