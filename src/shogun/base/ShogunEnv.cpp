/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Soeren Sonnenburg, Thoralf Klein, Pan Deng, Evgeniy Andreev,
 *          Viktor Gal, Giovanni De Toni, Heiko Strathmann, Bjoern Esser
 */

#include <shogun/base/ShogunEnv.h>
#include <shogun/io/fs/FileSystem.h>
#include <shogun/io/fs/FileSystemRegistry.h>

#include <shogun/io/SGIO.h>
#include <shogun/lib/Signal.h>
#include <shogun/mathematics/linalg/SGLinalg.h>

#include <rxcpp/rx-lite.hpp>

#include <csignal>
#include <functional>
#include <string>

#ifdef HAVE_PROTOBUF
#include <google/protobuf/stubs/common.h>
#endif

using namespace shogun;

ShogunEnv* ShogunEnv::instance()
{
	static auto shogun_env = new ShogunEnv();
	return shogun_env;
}

ShogunEnv::ShogunEnv()
{
	sg_io = new SGIO();
	sg_linalg = std::make_unique<SGLinalg>();
	sg_signal = std::make_unique<CSignal>();

	SG_REF(sg_io);

	sg_fequals_epsilon = 0.0;
	sg_fequals_tolerant = false;

	// Set up signal handler
	std::signal(SIGINT, sg_signal->handler);
	init_from_env();
}

ShogunEnv::~ShogunEnv()
{
	SG_UNREF(sg_io);
	delete CSignal::m_subscriber;
	delete CSignal::m_observable;
	delete CSignal::m_subject;

#ifdef HAVE_PROTOBUF
	::google::protobuf::ShutdownProtobufLibrary();
#endif
}

void ShogunEnv::init_from_env()
{
	char* env_log_val = NULL;
	env_log_val = getenv("SHOGUN_LOG_LEVEL");
	if (env_log_val)
	{
		if (strncmp(env_log_val, "DEBUG", 5) == 0)
			sg_io->set_loglevel(MSG_DEBUG);
		else if (strncmp(env_log_val, "WARN", 4) == 0)
			sg_io->set_loglevel(MSG_WARN);
		else if (strncmp(env_log_val, "ERROR", 5) == 0)
			sg_io->set_loglevel(MSG_ERROR);
	}

	char* env_warnings_val = NULL;
	env_warnings_val = getenv("SHOGUN_GPU_WARNINGS");
	if (env_warnings_val)
	{
		if (strncmp(env_warnings_val, "off", 3) == 0)
			sg_linalg->set_linalg_warnings(false);
	}

	char* env_thread_val = NULL;
	env_thread_val = getenv("SHOGUN_NUM_THREADS");
	if (env_thread_val)
	{
		try
		{
			set_num_threads(std::stoi(std::string(env_thread_val)));
		}
		catch (...)
		{
			auto& io = sg_io;
			SG_WARNING(
			    "The specified SHOGUN_NUM_THREADS environment (%s)"
			    "variable could not be parsed as integer!\n",
			    env_thread_val);
		}
	}
}

void ShogunEnv::set_global_io(SGIO* io)
{
	SG_REF(io);
	SG_UNREF(sg_io);
	sg_io = io;
}

SGIO* ShogunEnv::io()
{
	return sg_io;
}

float64_t ShogunEnv::fequals_epsilon()
{
	return sg_fequals_epsilon;
}

void ShogunEnv::set_global_fequals_epsilon(float64_t fequals_epsilon)
{
	sg_fequals_epsilon = fequals_epsilon;
}

void ShogunEnv::set_global_fequals_tolerant(bool fequals_tolerant)
{
	sg_fequals_tolerant = fequals_tolerant;
}

bool ShogunEnv::fequals_tolerant()
{
	return sg_fequals_tolerant;
}

CSignal* ShogunEnv::signal()
{
	return sg_signal.get();
}

SGLinalg* ShogunEnv::linalg()
{
	return sg_linalg.get();
}
