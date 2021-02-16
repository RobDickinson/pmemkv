// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2017-2021, Intel Corporation */

#include "unittest.hpp"

/**
 * Tests adding, reading and removing data; basic short, tests
 */

using namespace pmem::kv;

static void SimpleTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.exists(entry_from_string("key1")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &value), status::NOT_FOUND);
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.exists(entry_from_string("key1")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value1"));
	value = "";
	UT_ASSERT(kv.get(entry_from_string("key1"), [&](string_view v) {
		value.append(v.data(), v.size());
	}) == status::OK);
	UT_ASSERT(value == entry_from_string("value1"));
}

static void EmptyKeyTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.put(entry_from_string(""), entry_from_string("empty")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.put(entry_from_string(" "), entry_from_string("1-space")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 2);
	ASSERT_STATUS(kv.put(entry_from_string("\t\t"), entry_from_string("two-tab")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 3);
	std::string value1;
	std::string value2;
	std::string value3;
	ASSERT_STATUS(kv.exists(entry_from_string("")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string(""), &value1), status::OK);
	UT_ASSERT(value1 == entry_from_string("empty"));
	ASSERT_STATUS(kv.exists(entry_from_string(" ")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string(" "), &value2), status::OK);
	UT_ASSERT(value2 == entry_from_string("1-space"));
	ASSERT_STATUS(kv.exists(entry_from_string("\t\t")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string("\t\t"), &value3), status::OK);
	UT_ASSERT(value3 == entry_from_string("two-tab"));
}

static void EmptyValueTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.put(entry_from_string("empty"), entry_from_string("")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.put(entry_from_string("1-space"), entry_from_string(" ")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 2);
	ASSERT_STATUS(kv.put(entry_from_string("two-tab"), entry_from_string("\t\t")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 3);
	std::string value1;
	std::string value2;
	std::string value3;
	ASSERT_STATUS(kv.get(entry_from_string("empty"), &value1), status::OK);
	UT_ASSERT(value1 == entry_from_string(""));
	ASSERT_STATUS(kv.get(entry_from_string("1-space"), &value2), status::OK);
	UT_ASSERT(value2 == entry_from_string(" "));
	ASSERT_STATUS(kv.get(entry_from_string("two-tab"), &value3), status::OK);
	UT_ASSERT(value3 == entry_from_string("\t\t"));
}

static void EmptyKeyAndValueTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);

	std::string value = "abc";
	ASSERT_STATUS(kv.get(entry_from_string(""), &value), status::NOT_FOUND);
	UT_ASSERT(value == "abc");

	ASSERT_STATUS(kv.put(entry_from_string(""), entry_from_string("")), status::OK);
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);

	ASSERT_STATUS(kv.get(entry_from_string(""), &value), status::OK);
	UT_ASSERT(value == entry_from_string(""));
}

static void GetClearExternalValueTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("cool")),
		      status::OK);
	std::string value = "super";
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("cool"));

	value = "super";
	ASSERT_STATUS(kv.get(entry_from_string("nope"), &value), status::NOT_FOUND);
	UT_ASSERT(value == "super");
}

static void GetHeadlessTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.exists(entry_from_string("waldo")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("waldo"), &value), status::NOT_FOUND);
}

static void GetMultipleTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.put(entry_from_string("abc"), entry_from_string("A1")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("def"), entry_from_string("B2")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("hij"), entry_from_string("C3")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("jkl"), entry_from_string("D4")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("mno"), entry_from_string("E5")),
		      status::OK);
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 5);
	ASSERT_STATUS(kv.exists(entry_from_string("abc")), status::OK);
	std::string value1;
	ASSERT_STATUS(kv.get(entry_from_string("abc"), &value1), status::OK);
	UT_ASSERT(value1 == entry_from_string("A1"));
	ASSERT_STATUS(kv.exists(entry_from_string("def")), status::OK);
	std::string value2;
	ASSERT_STATUS(kv.get(entry_from_string("def"), &value2), status::OK);
	UT_ASSERT(value2 == entry_from_string("B2"));
	ASSERT_STATUS(kv.exists(entry_from_string("hij")), status::OK);
	std::string value3;
	ASSERT_STATUS(kv.get(entry_from_string("hij"), &value3), status::OK);
	UT_ASSERT(value3 == entry_from_string("C3"));
	ASSERT_STATUS(kv.exists(entry_from_string("jkl")), status::OK);
	std::string value4;
	ASSERT_STATUS(kv.get(entry_from_string("jkl"), &value4), status::OK);
	UT_ASSERT(value4 == entry_from_string("D4"));
	ASSERT_STATUS(kv.exists(entry_from_string("mno")), status::OK);
	std::string value5;
	ASSERT_STATUS(kv.get(entry_from_string("mno"), &value5), status::OK);
	UT_ASSERT(value5 == entry_from_string("E5"));
}

static void GetMultiple2Test(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("key2"), entry_from_string("value2")),
		      status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("key3"), entry_from_string("value3")),
		      status::OK);
	ASSERT_STATUS(kv.remove(entry_from_string("key2")), status::OK);
	ASSERT_STATUS(kv.put(entry_from_string("key3"), entry_from_string("VALUE3")),
		      status::OK);
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 2);
	std::string value1;
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &value1), status::OK);
	UT_ASSERT(value1 == entry_from_string("value1"));
	std::string value2;
	ASSERT_STATUS(kv.get(entry_from_string("key2"), &value2), status::NOT_FOUND);
	std::string value3;
	ASSERT_STATUS(kv.get(entry_from_string("key3"), &value3), status::OK);
	UT_ASSERT(value3 == entry_from_string("VALUE3"));
}

static void GetNonexistentTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);
	ASSERT_STATUS(kv.exists(entry_from_string("waldo")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("waldo"), &value), status::NOT_FOUND);
}

static void PutTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);

	std::string value;
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value1"));

	std::string new_value;
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("VALUE1")),
		      status::OK); // same size
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &new_value), status::OK);
	UT_ASSERT(new_value == entry_from_string("VALUE1"));

	std::string new_value2;
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("new_value")),
		      status::OK); // longer size
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &new_value2), status::OK);
	UT_ASSERT(new_value2 == entry_from_string("new_value"));

	std::string new_value3;
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("?")),
		      status::OK); // shorter size
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.get(entry_from_string("key1"), &new_value3), status::OK);
	UT_ASSERT(new_value3 == entry_from_string("?"));
}

static void PutValuesOfDifferentSizesTest(pmem::kv::db &kv)
{
	std::string value;
	ASSERT_STATUS(kv.put(entry_from_string("A"), entry_from_string("123456789ABCDE")),
		      status::OK);
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.get(entry_from_string("A"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("123456789ABCDE"));

	std::string value2;
	ASSERT_STATUS(
		kv.put(entry_from_string("B"), entry_from_string("123456789ABCDEF")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 2);
	ASSERT_STATUS(kv.get(entry_from_string("B"), &value2), status::OK);
	UT_ASSERT(value2 == entry_from_string("123456789ABCDEF"));

	std::string value3;
	ASSERT_STATUS(
		kv.put(entry_from_string("C"), entry_from_string("123456789ABCDEFG")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 3);
	ASSERT_STATUS(kv.get(entry_from_string("C"), &value3), status::OK);
	UT_ASSERT(value3 == entry_from_string("123456789ABCDEFG"));

	std::string value4;
	ASSERT_STATUS(
		kv.put(entry_from_string("D"), entry_from_string("123456789ABCDEFGH")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 4);
	ASSERT_STATUS(kv.get(entry_from_string("D"), &value4), status::OK);
	UT_ASSERT(value4 == entry_from_string("123456789ABCDEFGH"));

	std::string value5;
	ASSERT_STATUS(
		kv.put(entry_from_string("E"), entry_from_string("123456789ABCDEFGHI")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 5);
	ASSERT_STATUS(kv.get(entry_from_string("E"), &value5), status::OK);
	UT_ASSERT(value5 == entry_from_string("123456789ABCDEFGHI"));
}

static void RemoveAllTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.put(entry_from_string("tmpkey"), entry_from_string("tmpvalue1")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.remove(entry_from_string("tmpkey")), status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey"), &value), status::NOT_FOUND);
}

static void RemoveAndInsertTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.put(entry_from_string("tmpkey"), entry_from_string("tmpvalue1")),
		      status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.remove(entry_from_string("tmpkey")), status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey"), &value), status::NOT_FOUND);
	ASSERT_STATUS(
		kv.put(entry_from_string("tmpkey1"), entry_from_string("tmpvalue1")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey1")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey1"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("tmpvalue1"));
	ASSERT_STATUS(kv.remove(entry_from_string("tmpkey1")), status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey1")), status::NOT_FOUND);
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey1"), &value), status::NOT_FOUND);
}

static void RemoveExistingTest(pmem::kv::db &kv)
{
	std::size_t cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 0);
	ASSERT_STATUS(
		kv.put(entry_from_string("tmpkey1"), entry_from_string("tmpvalue1")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(
		kv.put(entry_from_string("tmpkey2"), entry_from_string("tmpvalue2")),
		status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 2);
	ASSERT_STATUS(kv.remove(entry_from_string("tmpkey1")), status::OK);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.remove(entry_from_string("tmpkey1")), status::NOT_FOUND);
	cnt = std::numeric_limits<std::size_t>::max();
	ASSERT_STATUS(kv.count_all(cnt), status::OK);
	UT_ASSERT(cnt == 1);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey1")), status::NOT_FOUND);
	std::string value;
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey1"), &value), status::NOT_FOUND);
	ASSERT_STATUS(kv.exists(entry_from_string("tmpkey2")), status::OK);
	ASSERT_STATUS(kv.get(entry_from_string("tmpkey2"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("tmpvalue2"));
}

static void RemoveHeadlessTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.remove(entry_from_string("nada")), status::NOT_FOUND);
}

static void RemoveNonexistentTest(pmem::kv::db &kv)
{
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);
	ASSERT_STATUS(kv.remove(entry_from_string("nada")), status::NOT_FOUND);
	ASSERT_STATUS(kv.exists(entry_from_string("key1")), status::OK);
}

static void MoveDBTest(pmem::kv::db &kv)
{
	/**
	 * TEST: test db constructor from another instance of db class
	 *	and move assignment operator (from different and the same db).
	 */

	/* put key1 in original db */
	ASSERT_STATUS(kv.put(entry_from_string("key1"), entry_from_string("value1")),
		      status::OK);

	db kv_new(std::move(kv));

	ASSERT_STATUS(kv_new.put(entry_from_string("key2"), entry_from_string("value2")),
		      status::OK);
	std::string value;
	value = "ABC";

	ASSERT_STATUS(kv_new.get(entry_from_string("key1"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value1"));
	ASSERT_STATUS(kv_new.get(entry_from_string("key2"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value2"));
	ASSERT_STATUS(kv_new.remove(entry_from_string("key1")), status::OK);

	db kv_assign_new = std::move(kv_new);
	auto &kv_assign_new2 = kv_assign_new;
	kv_assign_new = std::move(kv_assign_new2);

	ASSERT_STATUS(
		kv_assign_new.put(entry_from_string("key3"), entry_from_string("value3")),
		status::OK);

	ASSERT_STATUS(kv_assign_new.get(entry_from_string("key2"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value2"));
	ASSERT_STATUS(kv_assign_new.get(entry_from_string("key3"), &value), status::OK);
	UT_ASSERT(value == entry_from_string("value3"));

	ASSERT_STATUS(kv_assign_new.remove(entry_from_string("key2")), status::OK);
	ASSERT_STATUS(kv_assign_new.remove(entry_from_string("key3")), status::OK);

	kv_assign_new.close();
}

static void test(int argc, char *argv[])
{
	if (argc < 3)
		UT_FATAL("usage: %s engine json_config", argv[0]);

	run_engine_tests(
		argv[1], argv[2],
		{
			SimpleTest,
			EmptyKeyTest,
			EmptyValueTest,
			EmptyKeyAndValueTest,
			GetClearExternalValueTest,
			GetHeadlessTest,
			GetMultipleTest,
			GetMultiple2Test,
			GetNonexistentTest,
			PutTest,
			PutValuesOfDifferentSizesTest,
			RemoveAllTest,
			RemoveAndInsertTest,
			RemoveExistingTest,
			RemoveHeadlessTest,
			RemoveNonexistentTest,
			/* move DB test has to be the last one; it invalidates kv */
			MoveDBTest,
		});
}

int main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
