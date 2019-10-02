# pmemkv

[![Build Status](https://travis-ci.org/pmem/pmemkv.svg?branch=master)](https://travis-ci.org/pmem/pmemkv)
[![PMEMKV version](https://img.shields.io/github/tag/pmem/pmemkv.svg)](https://github.com/pmem/pmemkv/releases/latest)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/18408/badge.svg)](https://scan.coverity.com/projects/pmem-pmemkv)
[![Coverage Status](https://codecov.io/github/pmem/pmemkv/coverage.svg?branch=master)](https://codecov.io/gh/pmem/pmemkv/branch/master)

Key/Value Datastore for Persistent Memory

Overview
--------

`pmemkv` is a local/embedded key-value datastore optimized for persistent memory.
Rather than being tied to a single language or backing implementation, `pmemkv`
provides different options for language bindings and storage engines.

There is also a small helper library `pmemkv_json_config` provided.
See its [manual](doc/libpmemkv_json_config.3.md) for details.

<ul>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md">Installation</a></li>
<li><a href="#bindings">Language Bindings</a></li>
<li><a href="#engines">Storage Engines</a></li>
<li><a href="#tools">Tools and Utilities</a></li>
</ul>

<a name="installation"></a>

Installation
------------

<a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md">Installation</a> guide
provides detailed instructions how to build and install `pmemkv` from sources,
build rpm and deb packages and explains usage of experimental engines and pool sets.

<ul>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#building_from_sources">Building From Sources</a></li>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#fedora">Installing on Fedora</a></li>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#ubuntu">Installing on Ubuntu</a></li>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#build_package">Building packages</a></li>
<li><a href="https://github.com/pmem/pmemkv/blob/master/INSTALLING.md#experimental">Using Experimental Engines</a></li>
</ul>

<a name="bindings"></a>

Language Bindings
-----------------

`pmemkv` is written in C/C++ and it is used by bindings for Java, Node.js,
Python, and Ruby applications.

![pmemkv-bindings](https://user-images.githubusercontent.com/12031346/65962933-ff6bfc00-e459-11e9-9552-d6326e9c0684.png)

### C/C++ Examples

Examples for C and C++ can be found within this repository in [examples directory](https://github.com/pmem/pmemkv/tree/master/examples).

### Other Languages

Abovementioned bindings are maintained in separate GitHub repos, but are still kept
in sync with the main `pmemkv` distribution.

* Java - https://github.com/pmem/pmemkv-java
    * \+ Java Native Interface - https://github.com/pmem/pmemkv-jni
* Node.js - https://github.com/pmem/pmemkv-nodejs
* Python - https://github.com/pmem/pmemkv-python
* Ruby - https://github.com/pmem/pmemkv-ruby

<a name="engines"></a>

Storage Engines
---------------

`pmemkv` provides multiple storage engines that conform to the same common API, so every engine can be used with
all language bindings and utilities. Engines are loaded by name at runtime.

| Engine Name  | Description | Experimental? | Concurrent? | Sorted? |
| ------------ | ----------- | ------------- | ----------- | ------- |
| [blackhole](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#blackhole) | Accepts everything, returns nothing | No | Yes | No |
| [cmap](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#cmap) | Concurrent hash map | No | Yes | No |
| [vsmap](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#vsmap) | Volatile sorted hash map | No | No | Yes |
| [vcmap](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#vcmap) | Volatile concurrent hash map | No | Yes | No |
| [tree3](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#tree3) | Persistent B+ tree | Yes | No | No |
| [stree](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#stree) | Sorted persistent B+ tree | Yes | No | Yes |
| [caching](https://github.com/pmem/pmemkv/blob/master/ENGINES.md#caching) | Caching for remote Memcached or Redis server | Yes | No | - |

The production quality engines are described in the [libpmemkv(7)](https://pmem.io/pmemkv/master/manpages/libpmemkv.7.html) manual
and the experimental engines are described in the [ENGINES-experimental.md](https://github.com/pmem/pmemkv/blob/master/ENGINES-experimental.md) file.

[Contributing a new engine](https://github.com/pmem/pmemkv/blob/master/CONTRIBUTING.md#engines) is easy and encouraged!

<a name="tools"></a>

Tools and Utilities
-------------------

Benchmarks' scripts and other helpful utilities are available here:

https://github.com/pmem/pmemkv-tools
