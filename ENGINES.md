# Storage Engines for pmemkv

<ul>
<li><a href="#blackhole">blackhole</a></li>
<li><a href="#cmap">cmap</a></li>
<li><a href="#vsmap">vsmap</a></li>
<li><a href="#vcmap">vcmap</a></li>
<li><a href="#tree3">tree3</a></li>
<li><a href="#stree">stree</a></li>
<li><a href="#caching">caching</a></li>
</ul>

<a name="blackhole"></a>

blackhole
---------

A volatile concurrent engine that accepts an unlimited amount of data, but never returns anything.
It is always enabled (no CMake option is specified to enable/disable this engine).

* `Put` and `Remove` always returns `OK`
* `Get` always returns `NOT_FOUND`
* `All` and `Each` never trigger callbacks

### Configuration

No required configuration parameters.  JSON configuration is never parsed.

### Internals

Internally, `blackhole` does not use a persistent pool or any durable structures. The intended
use of this engine is to profile and tune high-level bindings, and similar cases when persistence
should be intentionally skipped.

### Prerequisites

No additional packages are required.

<a name="cmap"></a>

cmap
-----

A persistent concurrent engine, backed by a hashmap.
It is enabled by default. It can be disabled in CMake using the `ENGINE_CMAP` option.

### Configuration

Configuration must specify a `path` to a PMDK persistent pool, which can be a file (on a DAX filesystem),
a DAX device, or a PMDK poolset file.

```
{ "path" : "my-pool" }
```

Configuration may optionally specify a `size` in bytes.
If omitted the default value of 1073741824 bytes (1GB) is applied.
This value cannot be smaller than 8388608 (8MB).

```
{ "path" : "my-pool", "size" : 1073741824 }
```

### Internals

Internally the engine uses persistent concurrent hash map and persistent string from [libpmemobj-cpp](https://github.com/pmem/libpmemobj-cpp) library. Persistent string is used as a type of a key and a value.

### Prerequisites

TBB and libpmemobj-cpp packages are required.

<a name="vsmap"></a>

vsmap
-----

A volatile single-threaded sorted engine, backed by memkind.
It is enabled by default. It can be disabled in CMake using the `ENGINE_VSMAP` option.

### Configuration

Configuration must specify a `path` to a local directory where temporary files will be created.
For best performance this directory should reside on a DAX-enabled filesystem.

```
{ "path" : "my-directory" }
```

Configuration may optionally specify a `size` in bytes.
If omitted the default value of 1073741824 bytes (1GB) is applied.
This value cannot be smaller than 8388608 (8MB).

```
{ "path" : "my-directory", "size" : 1073741824 }
```

### Internals

The engine is built on top of [std::map](https://en.cppreference.com/w/cpp/container/map). The map uses [PMEM C++ allocator](https://github.com/memkind/memkind/blob/master/include/pmem_allocator.h) to allocate memory. The [std::basic_string](https://en.cppreference.com/w/cpp/string/basic_string) with [PMEM C++ allocator](https://github.com/memkind/memkind/blob/master/include/pmem_allocator.h) is used as a type of a key and a value.

### Prerequisites

Memkind and libpmemobj-cpp packages are required.

<a name="vcmap"></a>

vcmap
-----

A volatile concurrent engine, backed by memkind.
It is enabled by default. It can be disabled in CMake using the `ENGINE_VCMAP` option.

### Configuration

(same as the `vsmap` engine)

### Internals

The engine is built on top of [tbb::concurrent_hash_map](https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_hash_map_cls.html) data structure. The hash map uses [PMEM C++ allocator](https://github.com/memkind/memkind/blob/master/include/pmem_allocator.h) to allocate memory. The [std::basic_string](https://en.cppreference.com/w/cpp/string/basic_string) with [PMEM C++ allocator](https://github.com/memkind/memkind/blob/master/include/pmem_allocator.h) is used as a type of a key and a value.

### Prerequisites

Memkind, TBB and libpmemobj-cpp packages are required.

<a name="tree3"></a>

tree3
-----

A persistent single-threaded engine, backed by a read-optimized B+ tree.
It is disabled by default. It can be enabled in CMake using the `ENGINE_TREE3` option.

### Configuration

Configuration must specify a `path` to a PMDK persistent pool, which can be a file (on a DAX filesystem),
a DAX device, or a PMDK poolset file.

```
{ "path" : "my-pool" }
```

Configuration may optionally specify a `size` in bytes.
If omitted the default value of 1073741824 bytes (1GB) is applied.
This value cannot be smaller than 8388608 (8MB).

```
{ "path" : "my-pool", "size" : 1073741824 }
```

### Internals

Internally, `tree3` uses a hybrid fingerprinted B+ tree implementation. Rather than keeping
inner and leaf nodes of the tree in persistent memory, `tree3` uses a hybrid structure where
inner nodes are kept in DRAM and leaf nodes only are kept in persistent memory. Though `tree3`
has to recover all inner nodes when the engine is started, searches are performed in 
DRAM except for a final read from persistent memory.

![pmemkv-intro](https://cloud.githubusercontent.com/assets/913363/25543024/289f06d8-2c12-11e7-86e4-a1f0df891659.png)

Leaf nodes in `tree3` contain multiple key-value pairs, indexed using 1-byte fingerprints
([Pearson hashes](https://en.wikipedia.org/wiki/Pearson_hashing)) that speed locating
a given key. Leaf modifications are accelerated using
[zero-copy updates](http://pmem.io/2017/03/09/pmemkv-zero-copy-leaf-splits.html).

### Prerequisites

Libpmemobj-cpp package is required.

<a name="stree"></a>

stree
---------

(TBD)
It is disabled by default. It can be enabled in CMake using the `ENGINE_STREE` option.

### Configuration

(TBD)

### Internals

(TBD)

### Prerequisites

Libpmemobj-cpp package is required.

<a name="caching"></a>

caching
---------

(TBD)
It is disabled by default. It can be enabled in CMake using the `ENGINE_CACHING` option.

### Configuration

(TBD)

### Internals

(TBD)

### Prerequisites

Memcached and libacl packages required (<a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#experimental">see here for installation guide</a>).

### Related Work
---------

**pmse**

`tree3` has a lot in common with [pmse](https://github.com/pmem/pmse)
-- both implementations rely on PMDK internally, although
they expose different APIs externally. Both `pmse` and `tree3` are based on a B+ tree
implementation. The biggest difference is that the `pmse`
tree keeps inner and leaf nodes in persistent memory,
where `tree3` keeps inner nodes in DRAM and leaf nodes in
persistent memory. (This means that `tree3` has to recover
all inner nodes when the engine is started)

**FPTree**

This research paper describes a hybrid DRAM/NVM tree design (similar
to the `tree3` storage engine) but this paper doesn't provide any code, and 
omits certain important implementation details.

Beyond providing a clean-room implementation, the design of `tree3`
differs from FPTree in several important areas:

1. `tree3` is written using PMDK C++ bindings, which exerts influence on
its design and implementation. `tree3` uses generic PMDK transactions
(i.e. `transaction::run()` closures), there is no need for micro-logging
structures as described in the FPTree paper to make internal delete and
split operations safe. `tree3` also adjusts sizes of data structures
(to fit PMDK primitive types) for best cache-line optimization.

2. FPTree does not specify a hash method implementation, where `tree3`
uses a Pearson hash (RFC 3074).

3. Within its persistent leaves, FPTree uses an array of key hashes with
a separate visibility bitmap to track what hash slots are occupied.
`tree3` takes a different approach and uses key hashes themselves to track
visibility. This relies on a specially modified Pearson hash function,
where a hash value of zero always indicates the slot is unused.
This optimization eliminates the cost of using and maintaining
visibility bitmaps as well as cramming more hashes into a single
cache-line, and affects the implementation of every primitive operation
in the tree.

4. `tree3` caches key hashes in DRAM (in addition to storing these as
part of the persistent leaf). This speeds leaf operations, especially with
slower media, for what seems like an acceptable rise in DRAM usage.

5. Within its persistent leaves, `tree3` combines hash, key and value
into a single slot type (`KVSlot`). This leads to improved leaf split
performance and reduced write amplification, since splitting can be
performed by swapping pointers to slots without copying any key or
value data stored in the slots. `KVSlot` internally stores key and
value to a single persistent buffer, which minimizes the number of
persistent allocations and improves storage efficiency with larger 
keys and values.

**cpp_map**

Use of PMDK C++ bindings by `tree3` was lifted from this example program.
Many thanks to [@tomaszkapela](https://github.com/tomaszkapela)
for providing a great example to follow!
