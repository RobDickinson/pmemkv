// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * concurrent_hash_map_reorder.cpp -- pmem::obj::concurrent_hash_map test
 *
 */

#include "unittest.hpp"

static std::string elements[] = {
	"A", /* bucket #1 */
	"B", /* bucket #2 */
	"C", /* bucket #3 */
	"D", /* bucket #1 */
	"E", /* bucket #2 */
	"F", /* bucket #3 */
};

static constexpr int len_elements =
	static_cast<int>(sizeof(elements) / sizeof(elements[0]));

static void check_exist(pmem::kv::db &kv, const std::string &element,
			pmem::kv::status exists)
{
	std::string value;
	UT_ASSERT(kv.get(element, &value) == exists);

	if (exists == pmem::kv::status::OK) {
		UT_ASSERT(element == value);
	}
}

static void test_init(pmem::kv::db &kv)
{
	for (int i = 0; i < len_elements / 2; i++) {
		kv.put(elements[i], elements[i]);
		check_exist(kv, elements[i], pmem::kv::status::OK);
	}
}

static void test_insert(pmem::kv::db &kv)
{
	for (int i = len_elements / 2; i < len_elements - 1; i++) {
		kv.put(elements[i], elements[i]);
		check_exist(kv, elements[i], pmem::kv::status::OK);
	}
}

static void check_consistency(pmem::kv::db &kv)
{
	std::size_t cnt;
	kv.count_all(cnt);
	int size = static_cast<int>(cnt);

	for (int i = 0; i < size; i++) {
		check_exist(kv, elements[i], pmem::kv::status::OK);
	}

	for (int i = size; i < len_elements; i++) {
		check_exist(kv, elements[i], pmem::kv::status::NOT_FOUND);
	}
}

static void test(int argc, char *argv[])
{
	std::cout << "ARGC: " << argc << std::endl;
	for (int i = 0; i < argc; ++i) {
		std::cout << "ARGV " << i << " : " << argv[i] << std::endl;
	}
	if (argc < 4)
		UT_FATAL("usage: %s engine json_config <create|open|insert>", argv[0]);

	std::string mode = argv[3];
	if (mode != "create" && mode != "open" && mode != "insert")
		UT_FATAL("usage: %s engine json_config <create|open|insert>", argv[0]);

	auto kv = INITIALIZE_KV(argv[1], CONFIG_FROM_JSON(argv[2]));

	if (mode == "create") {
		test_init(kv);
	} else if (mode == "open") {
		check_consistency(kv);
	} else if (mode == "insert") {
		test_insert(kv);
	}

	kv.close();
}

int main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
