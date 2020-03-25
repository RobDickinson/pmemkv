// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2017-2020, Intel Corporation */

#include "unittest.hpp"

#include <algorithm>
#include <vector>

using namespace pmem::kv;

using test_kv = std::pair<std::string, std::string>;
using test_kv_list = std::vector<test_kv>;

static test_kv_list sort(test_kv_list list)
{
	std::sort(list.begin(), list.end(), [](const test_kv &lhs, const test_kv &rhs) {
		return lhs.first < rhs.first;
	});

	return list;
}

static void UsesGetAllTest(pmem::kv::db &kv)
{
	UT_ASSERT(kv.put("1", "2") == status::OK);
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	UT_ASSERT(kv.count_all(cnt) == status::OK);
	UT_ASSERT(cnt == 1);
	UT_ASSERT(kv.put("RR", "记!") == status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	UT_ASSERT(kv.count_all(cnt) == status::OK);
	UT_ASSERT(cnt == 2);

	test_kv_list result;
	kv.get_all(
		[](const char *k, size_t kb, const char *v, size_t vb, void *arg) {
			const auto c = ((test_kv_list *)arg);
			c->emplace_back(std::string(k, kb), std::string(v, vb));
			return 0;
		},
		&result);

	auto expected = test_kv_list{{"1", "2"}, {"RR", "记!"}};
	UT_ASSERT((sort(result) == sort(expected)));
}

static void UsesGetAllTest2(pmem::kv::db &kv)
{
	UT_ASSERT(kv.put("1", "one") == status::OK);
	UT_ASSERT(kv.put("2", "two") == status::OK);
	UT_ASSERT(kv.put("记!", "RR") == status::OK);

	test_kv_list result;
	kv.get_all([&](string_view k, string_view v) {
		result.emplace_back(std::string(k.data(), k.size()),
				    std::string(v.data(), v.size()));
		return 0;
	});

	auto expected = test_kv_list{{"1", "one"}, {"2", "two"}, {"记!", "RR"}};
	UT_ASSERT((sort(result) == sort(expected)));

	result = {};
	kv.get_all(
		[](const char *k, size_t kb, const char *v, size_t vb, void *arg) {
			const auto c = ((test_kv_list *)arg);
			c->emplace_back(std::string(k, kb), std::string(v, vb));
			return 0;
		},
		&result);

	expected = test_kv_list{{"1", "one"}, {"2", "two"}, {"记!", "RR"}};
	UT_ASSERT((sort(result) == sort(expected)));
}

static void test(int argc, char *argv[])
{
	if (argc < 3)
		UT_FATAL("usage: %s engine json_config", argv[0]);

	run_engine_tests(argv[1], argv[2], {UsesGetAllTest, UsesGetAllTest2});
}

int main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
