// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

#include "unittest.hpp"

#include <ctime>

using namespace pmem::kv;

void generate_keys(std::vector<std::string> &keys, const size_t max_key_len,
		   const size_t cnt)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyz"
			       "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
			       "!@#$%^&*()_+,./<>?'\"`~;:[]{}\\|";
	const size_t max_index = (sizeof(charset) - 1);

	for (size_t k = 0; k < cnt; k++) {
		std::string gen_key;
		/* various lenght of key, min: 1 */
		size_t key_len = 1 + ((size_t)rand() % (max_key_len - 1));
		for (size_t i = 0; i < key_len; i++) {
			gen_key.push_back(charset[(size_t)rand() % max_index]);
		}
		keys.push_back(gen_key);
	}
}

static void MultithreadedTestRemoveDataAside(const size_t threads_number,
					     const size_t thread_items,
					     const size_t max_key_len, pmem::kv::db &kv)
{
	size_t initial_count = 128;
	/* put initial data, which won't be touched */
	for (size_t i = 0; i < initial_count; i++) {
		std::string istr = "init_" + std::to_string(i);
		UT_ASSERT(kv.put(istr, (istr + "!")) == status::OK);
	}

	std::vector<std::string> keys;
	auto keys_cnt = threads_number * thread_items;
	generate_keys(keys, max_key_len, keys_cnt);

	/* test parallelly adding data */
	parallel_exec(threads_number, [&](size_t thread_id) {
		size_t begin = thread_id * thread_items;
		size_t end = begin + thread_items;
		for (auto i = begin; i < end; i++) {
			std::string istr = std::to_string(i);
			UT_ASSERT(kv.put((istr + keys[i]), (istr + "!")) == status::OK);
		}
		for (auto i = begin; i < end; i++) {
			std::string istr = std::to_string(i);
			std::string value;
			UT_ASSERT(kv.get((istr + keys[i]), &value) == status::OK &&
				  value == (istr + "!"));
		}
	});

	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	UT_ASSERT(kv.count_all(cnt) == status::OK);
	UT_ASSERTeq(cnt, initial_count + keys_cnt);

	/* test parallelly removing data */
	parallel_exec(threads_number, [&](size_t thread_id) {
		size_t begin = thread_id * thread_items;
		size_t end = begin + thread_items;
		for (auto i = begin; i < end; i++) {
			std::string istr = std::to_string(i);
			UT_ASSERT(kv.remove((istr + keys[i])) == status::OK);
		}
	});
	cnt = std::numeric_limits<std::size_t>::max();
	UT_ASSERT(kv.count_all(cnt) == status::OK && cnt == initial_count);

	/* get initial data and confirm it's untouched */
	for (size_t i = 0; i < initial_count; i++) {
		std::string istr = "init_" + std::to_string(i);
		std::string value;
		UT_ASSERT(kv.get(istr, &value) == status::OK && value == (istr + "!"));
	}
}

static void test(int argc, char *argv[])
{
	using namespace std::placeholders;

	if (argc < 6)
		UT_FATAL("usage: %s engine json_config threads items max_key_len",
			 argv[0]);

	auto seed = unsigned(std::time(0));
	printf("rand seed: %u\n", seed);
	std::srand(seed);

	size_t threads_number = std::stoull(argv[3]);
	size_t thread_items = std::stoull(argv[4]);
	size_t max_key_len = std::stoull(argv[5]);
	run_engine_tests(argv[1], argv[2],
			 {
				 std::bind(MultithreadedTestRemoveDataAside,
					   threads_number, thread_items, max_key_len, _1),
			 });
}

int main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
