// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

#include "../all/put_get_std_map.hpp"

#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>

struct Root {
	PMEMoid oid;
};

static void test(int argc, char *argv[])
{
	using namespace std::placeholders;

	if (argc < 6)
		UT_FATAL("usage: %s engine path n_inserts key_length value_length",
			 argv[0]);

	auto n_inserts = std::stoull(argv[3]);
	auto key_length = std::stoull(argv[4]);
	auto value_length = std::stoull(argv[5]);

	auto pmemobj_pool_path = std::string(argv[2]);
	pmem::obj::pool<Root> pmemobj_pool;

	try {
		pmemobj_pool = pmem::obj::pool<Root>::open(pmemobj_pool_path, "pmemkv");
	} catch (std::exception &e) {
		UT_FATALexc(e);
	}

	pmem::kv::config cfg;
	auto s = cfg.put_object("oid", &pmemobj_pool.root()->oid, nullptr);
	UT_ASSERTeq(s, status::OK);

	auto kv = INITIALIZE_KV(argv[1], std::move(cfg));

	auto proto = PutToMapTest(n_inserts, key_length, value_length, kv);
	VerifyKv(proto, kv);

	kv.close();

	pmemobj_pool.close();
}

int main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
