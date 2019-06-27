/*
 * Copyright 2017-2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <unistd.h>

#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>

#include "../out.h"
#include "stree.h"

#define DO_LOG 0
#define LOG(msg)                                                                         \
	do {                                                                             \
		if (DO_LOG)                                                              \
			std::cout << "[stree] " << msg << "\n";                          \
	} while (0)

using pmem::detail::conditional_add_to_tx;
using pmem::obj::make_persistent_atomic;
using pmem::obj::transaction;

namespace pmem
{
namespace kv
{

stree::stree(std::unique_ptr<internal::config> cfg)
{
	const char *path;
	std::size_t size;

	if (cfg->get_string("path", &path) != status::OK)
		throw std::runtime_error("Config does not contain path");

	uint64_t force_create;
	auto ret = cfg->get_uint64("force_create", &force_create);

	if (ret == status::NOT_FOUND)
		force_create = 0;
	else if (ret != status::OK)
		throw std::runtime_error("Cannot get force_create from config");

	if (force_create) {
		if (cfg->get_uint64("size", &size) != status::OK)
			throw std::runtime_error("Config does not contain size");

		LOG("Creating filesystem pool, path=" << path << ", size="
						      << std::to_string(size));
		pmpool = pool<RootData>::create(path, LAYOUT, size, S_IRWXU);
	} else {
		LOG("Opening pool, path=" << path);
		pmpool = pool<RootData>::open(path, LAYOUT);
	}

	Recover();
	LOG("Started ok");
}

stree::~stree()
{
	LOG("Stopping");
	pmpool.close();
	LOG("Stopped ok");
}

std::string stree::name()
{
	return "stree";
}

status stree::count_all(std::size_t &cnt)
{
	std::size_t result = 0;
	for (auto &iterator : *my_btree)
		result++;

	cnt = result;

	return status::OK;
}

status stree::get_all(get_kv_callback *callback, void *arg)
{
	LOG("Each");
	for (auto &iterator : *my_btree) {
		auto ret = callback(iterator.first.c_str(), iterator.first.size(),
				    iterator.second.c_str(), iterator.second.size(), arg);
		if (ret != 0)
			return status::STOPPED_BY_CB;
	}

	return status::OK;
}

status stree::exists(string_view key)
{
	LOG("Exists for key=" << std::string(key.data(), key.size()));
	btree_type::iterator it = my_btree->find(pstring<20>(key.data(), key.size()));
	if (it == my_btree->end()) {
		LOG("  key not found");
		return status::NOT_FOUND;
	}
	return status::OK;
}

status stree::get(string_view key, get_v_callback *callback, void *arg)
{
	LOG("Get using callback for key=" << std::string(key.data(), key.size()));
	btree_type::iterator it = my_btree->find(pstring<20>(key.data(), key.size()));
	if (it == my_btree->end()) {
		LOG("  key not found");
		return status::NOT_FOUND;
	}

	callback(it->second.c_str(), it->second.size(), arg);
	return status::OK;
}

status stree::put(string_view key, string_view value)
{
	LOG("Put key=" << std::string(key.data(), key.size())
		       << ", value.size=" << std::to_string(value.size()));
	try {
		auto result = my_btree->insert(std::make_pair(
			pstring<MAX_KEY_SIZE>(key.data(), key.size()),
			pstring<MAX_VALUE_SIZE>(value.data(), value.size())));
		if (!result.second) { // key already exists, so update
			typename btree_type::value_type &entry = *result.first;
			transaction::manual tx(pmpool);
			conditional_add_to_tx(&(entry.second));
			entry.second = std::string(value.data(), value.size());
			transaction::commit();
		}
		return status::OK;
	} catch (std::bad_alloc e) {
		ERR() << "Put failed due to exception, " << e.what();
		return status::FAILED;
	} catch (pmem::transaction_alloc_error e) {
		ERR() << "Put failed due to pmem::transaction_alloc_error, " << e.what();
		return status::FAILED;
	} catch (pmem::transaction_error e) {
		ERR() << "Put failed due to pmem::transaction_error, " << e.what();
		return status::FAILED;
	}
}

status stree::remove(string_view key)
{
	LOG("Remove key=" << std::string(key.data(), key.size()));
	try {
		auto result = my_btree->erase(std::string(key.data(), key.size()));
		return (result == 1) ? status::OK : status::NOT_FOUND;
	} catch (std::bad_alloc e) {
		ERR() << "Put failed due to exception, " << e.what();
		return status::FAILED;
	} catch (pmem::transaction_alloc_error e) {
		ERR() << "Put failed due to pmem::transaction_alloc_error, " << e.what();
		return status::FAILED;
	} catch (pmem::transaction_error e) {
		ERR() << "Put failed due to pmem::transaction_error, " << e.what();
		return status::FAILED;
	}
}

void stree::Recover()
{
	auto root_data = pmpool.root();
	if (root_data->btree_ptr) {
		my_btree = root_data->btree_ptr.get();
		my_btree->garbage_collection();
	} else {
		make_persistent_atomic<btree_type>(pmpool, root_data->btree_ptr);
		my_btree = root_data->btree_ptr.get();
	}
}

} // namespace kv
} // namespace pmem
