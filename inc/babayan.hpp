#pragma once

#define BOOST_THREAD_VERSION 5

#include <string>
#include <iostream>
#include <set>
#include <memory>
#include <fstream>
#include <typeinfo>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

#include <boost/thread/thread_only.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <boost/coroutine2/all.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "config.h"
#include "keeper.h"
#include "hasher.h"
#include "reader.h"
#include "scaner.h"
