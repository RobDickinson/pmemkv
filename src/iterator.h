// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

#ifndef LIBPMEMKV_ITERATOR_H
#define LIBPMEMKV_ITERATOR_H

#include "libpmemkv.hpp"

#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/slice.hpp>
#include <libpmemobj++/transaction.hpp>

namespace pmem
{
namespace kv
{
namespace internal
{
class iterator_base {
public:
	virtual ~iterator_base() = default;

	virtual status seek(string_view key) = 0;
	virtual status seek_lower(string_view key);
	virtual status seek_lower_eq(string_view key);
	virtual status seek_higher(string_view key);
	virtual status seek_higher_eq(string_view key);

	virtual status seek_to_first();
	virtual status seek_to_last();

	virtual status next();
	virtual status prev();

	virtual std::pair<string_view, status> key() = 0;
	virtual std::pair<pmem::obj::slice<const char *>, status>
	read_range(size_t pos, size_t n) = 0;
};

class write_iterator_base : virtual public iterator_base {
public:
	virtual std::pair<pmem::obj::slice<char *>, status> write_range(size_t pos,
									size_t n) = 0;

	virtual status commit() = 0;
	void abort();

protected:
	std::vector<std::pair<std::string, size_t>> log;
	string_view snapshotted_key = {};
};

} /* namespace internal */
} /* namespace kv */
} /* namespace pmem */

#endif /* LIBPMEMKV_ITERATOR_H */
