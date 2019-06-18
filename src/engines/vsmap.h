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

#pragma once

#include "../engine.h"
#include "pmem_allocator.h"
#include <map>
#include <scoped_allocator>
#include <string>

namespace pmem
{
namespace kv
{

class vsmap : public engine_base {
public:
	vsmap(void *context, const std::string &path, size_t size);
	~vsmap();

	std::string name() final;
	void *engine_context();

	status all(all_callback *callback, void *arg) final;
	status all_above(string_view key, all_callback *callback, void *arg) final;
	status all_below(string_view key, all_callback *callback, void *arg) final;
	status all_between(string_view key1, string_view key2, all_callback *callback,
			   void *arg) final;

	status count(std::size_t &cnt) final;
	status count_above(string_view key, std::size_t &cnt) final;
	status count_below(string_view key, std::size_t &cnt) final;
	status count_between(string_view key1, string_view key2, std::size_t &cnt) final;

	status each(each_callback *callback, void *arg) final;
	status each_above(string_view key, each_callback *callback, void *arg) final;
	status each_below(string_view key, each_callback *callback, void *arg) final;
	status each_between(string_view key1, string_view key2, each_callback *callback,
			    void *arg) final;

	status exists(string_view key) final;

	status get(string_view key, get_callback *callback, void *arg) final;

	status put(string_view key, string_view value) final;

	status remove(string_view key) final;

private:
	using storage_type =
		std::basic_string<char, std::char_traits<char>, pmem::allocator<char>>;

	using key_type = storage_type;
	using mapped_type = storage_type;
	using map_allocator_type = pmem::allocator<std::pair<key_type, mapped_type>>;
	using map_type = std::map<key_type, mapped_type, std::less<key_type>,
				  std::scoped_allocator_adaptor<map_allocator_type>>;

	void *context;
	map_allocator_type kv_allocator;
	map_type pmem_kv_container;
};

} /* namespace kv */
} /* namespace pmem */
