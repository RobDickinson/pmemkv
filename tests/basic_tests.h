#/*
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
#ifndef BASIC_TESTS_H
#define BASIC_TESTS_H

#include "test_suite.h"

Basic basic_tests[] = {
#ifdef ENGINE_CACHING
	{
		.path = "/dev/shm",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 1,
		.engine = "caching",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmCaching100bKey100bValue",
	},
#endif // ENGINE_CHACHING
#ifdef ENGINE_TREE3
	{
		.path = "/dev/shm",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 1,
		.engine = "tree3",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmTree3100bKey100bValue",
	},
#endif // ENGINE_TREE3
#ifdef ENGINE_STREE
	{
		.path = "/dev/shm",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 1,
		.engine = "stree",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmStree100bKey100bValue",
	},
#endif // ENGINE_STREE
#ifdef ENGINE_CMAP
	{
		.path = "/dev/shm/file",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 0,
		.engine = "cmap",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmCmap100bKey100bValue",
	},
#endif // ENGINE_CMAP
#ifdef ENGINE_VSMAP
	{
		.path = "/dev/shm",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 1,
		.engine = "vsmap",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmVsmap100bKey100bValue",
	},
#endif // ENGINE_VSMAP
#ifdef ENGINE_VCMAP
	{
		.path = "/dev/shm",
		.size = (uint64_t)(1024 * 1024 * 1024),
		.force_create = 1,
		.engine = "vcmap",
		.key_length = 100,
		.value_length = 100,
		.test_data_size = 20,
		.name = "ShmVcmap100bKey100bValue",
	},
#endif // ENGINE_VCMAP
};
#endif // BASIC_TESTS_H_
