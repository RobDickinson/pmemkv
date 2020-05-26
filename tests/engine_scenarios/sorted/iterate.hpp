// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

#include "unittest.hpp"

#include <algorithm>
#include <vector>

/**
 * Helper class for testing get_* and count_* functions
 */

using namespace pmem::kv;
using kv_pair = std::pair<std::string, std::string>;
using kv_list = std::vector<kv_pair>;

const std::string EMPTY_KEY = "";
const std::string MIN_KEY = std::string(1, char(1));
const std::string MAX_KEY = std::string(4, char(255));
const std::string MID_KEY = std::string(2, char(127));

inline kv_list kv_sort(kv_list list)
{
	std::sort(list.begin(), list.end(), [](const kv_pair &lhs, const kv_pair &rhs) {
		return lhs.first < rhs.first;
	});

	return list;
}

#define KV_GET_ALL_CPP_CB_LST(list)                                                      \
	kv.get_all([&](string_view k, string_view v) {                                   \
		list.emplace_back(std::string(k.data(), k.size()),                       \
				  std::string(v.data(), v.size()));                      \
		return 0;                                                                \
	});

#define KV_GET_ALL_C_CB_LST(list)                                                        \
	kv.get_all(                                                                      \
		[](const char *k, size_t kb, const char *v, size_t vb, void *arg) {      \
			const auto c = ((kv_list *)arg);                                 \
			c->emplace_back(std::string(k, kb), std::string(v, vb));         \
			return 0;                                                        \
		},                                                                       \
		&list)

#define KV_GET_1KEY_CPP_CB_LST(func, key, list)                                          \
	kv.func(key, [&](string_view k, string_view v) {                                 \
		list.emplace_back(std::string(k.data(), k.size()),                       \
				  std::string(v.data(), v.size()));                      \
		return 0;                                                                \
	})

#define KV_GET_1KEY_C_CB_LST(func, key, list)                                            \
	kv.func(                                                                         \
		key,                                                                     \
		[](const char *k, size_t kb, const char *v, size_t vb, void *arg) {      \
			const auto c = ((kv_list *)arg);                                 \
			c->emplace_back(std::string(k, kb), std::string(v, vb));         \
			return 0;                                                        \
		},                                                                       \
		&list)

#define KV_GET_2KEYS_CPP_CB_LST(func, key1, key2, list)                                  \
	kv.func(key1, key2, [&](string_view k, string_view v) {                          \
		list.emplace_back(std::string(k.data(), k.size()),                       \
				  std::string(v.data(), v.size()));                      \
		return 0;                                                                \
	})

#define KV_GET_2KEYS_C_CB_LST(func, key1, key2, list)                                    \
	kv.func(                                                                         \
		key1, key2,                                                              \
		[](const char *k, size_t kb, const char *v, size_t vb, void *arg) {      \
			const auto c = ((kv_list *)arg);                                 \
			c->emplace_back(std::string(k, kb), std::string(v, vb));         \
			return 0;                                                        \
		},                                                                       \
		&list)

inline void verify_get_all(pmem::kv::db &kv, const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_all(cnt) == status::OK && cnt == exp_cnt);

	auto s = KV_GET_ALL_CPP_CB_LST(result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_all_c(pmem::kv::db &kv, const size_t exp_cnt,
			     const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_all(cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_ALL_C_CB_LST(result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_above(pmem::kv::db &kv, const std::string key,
			     const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_above(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_CPP_CB_LST(get_above, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_above_c(pmem::kv::db &kv, const std::string key,
			       const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_above(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_C_CB_LST(get_above, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_equal_above(pmem::kv::db &kv, const std::string key,
				   const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_equal_above(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_CPP_CB_LST(get_equal_above, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_equal_above_c(pmem::kv::db &kv, const std::string key,
				     const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_equal_above(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_C_CB_LST(get_equal_above, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_below(pmem::kv::db &kv, const std::string key,
			     const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_below(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_CPP_CB_LST(get_below, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_below_c(pmem::kv::db &kv, const std::string key,
			       const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_below(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_C_CB_LST(get_below, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_equal_below(pmem::kv::db &kv, const std::string key,
				   const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_equal_below(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_CPP_CB_LST(get_equal_below, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_equal_below_c(pmem::kv::db &kv, const std::string key,
				     const size_t exp_cnt, const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_equal_below(key, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_1KEY_C_CB_LST(get_equal_below, key, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_between(pmem::kv::db &kv, const std::string key1,
			       const std::string key2, const size_t exp_cnt,
			       const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_between(key1, key2, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_2KEYS_CPP_CB_LST(get_between, key1, key2, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

inline void verify_get_between_c(pmem::kv::db &kv, const std::string key1,
				 const std::string key2, const size_t exp_cnt,
				 const kv_list exp_lst)
{
	std::size_t cnt;
	kv_list result;
	UT_ASSERT(kv.count_between(key1, key2, cnt) == status::OK && cnt == exp_cnt);
	auto s = KV_GET_2KEYS_C_CB_LST(get_between, key1, key2, result);
	UT_ASSERTeq(s, status::OK);
	UT_ASSERT(result == kv_sort(exp_lst));
}

const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		       "abcdefghijklmnopqrstuvwxyz";
const size_t charset_size = strlen(charset);

inline void add_basic_keys(pmem::kv::db &kv)
{
	UT_ASSERTeq(kv.put("A", "1"), status::OK);
	UT_ASSERTeq(kv.put("AB", "2"), status::OK);
	UT_ASSERTeq(kv.put("AC", "3"), status::OK);
	UT_ASSERTeq(kv.put("B", "4"), status::OK);
	UT_ASSERTeq(kv.put("BB", "5"), status::OK);
	UT_ASSERTeq(kv.put("BC", "6"), status::OK);
}

inline void add_ext_keys(pmem::kv::db &kv)
{
	UT_ASSERT(kv.put("aaa", "1") == status::OK);
	UT_ASSERT(kv.put("bbb", "2") == status::OK);
	UT_ASSERT(kv.put("ccc", "3") == status::OK);
	UT_ASSERT(kv.put("rrr", "4") == status::OK);
	UT_ASSERT(kv.put("sss", "5") == status::OK);
	UT_ASSERT(kv.put("ttt", "6") == status::OK);
	UT_ASSERT(kv.put("yyy", "记!") == status::OK);
}

/* generates incremental keys using each char from global variable charset */
inline std::vector<std::string> gen_incr_keys(const size_t max_key_len)
{
	std::vector<std::string> keys;

	for (size_t i = 0; i < charset_size; i++) {
		std::string key = "";
		char curr_char = charset[i];
		for (size_t j = 0; j < max_key_len; j++) {
			key += curr_char;
			keys.push_back(key);
		}
	}

	return keys;
}

/* generates 'cnt' unique keys with random content, using global variable charset */
inline std::vector<std::string> gen_rand_keys(const size_t cnt, const size_t max_key_len)
{
	std::vector<std::string> keys;
	std::string gen_key;

	for (size_t k = 0; k < cnt; k++) {
		do {
			/* various lenght of key, min: 2 */
			size_t key_len = 2 + ((size_t)rand() % (max_key_len - 1));
			gen_key.clear();
			gen_key.reserve(key_len);
			for (size_t i = 0; i < key_len; i++) {
				gen_key.push_back(charset[(size_t)rand() % charset_size]);
			}
		} while (std::find(keys.begin(), keys.end(), gen_key) != keys.end());
		keys.push_back(std::move(gen_key));
	}

	return keys;
}
