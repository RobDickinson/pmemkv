/*
 * Copyright 2019, Intel Corporation
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

#ifndef LIBPMEMKV_H
#define LIBPMEMKV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "status.h"

typedef struct pmemkv_engine pmemkv_engine;

typedef void(pmemkv_all_callback_t)(void* context, int keybytes, const char* key);
typedef void(pmemkv_each_callback_t)(void* context, int keybytes, const char* key, int valuebytes, const char* value);
typedef void(pmemkv_get_callback_t)(void* context, int valuebytes, const char* value);
typedef void(pmemkv_start_failure_callback_t)(void* context, const char* engine, const char* config, const char* msg);

pmemkv_engine* pmemkv_start(void* context, const char* engine, const char* config, pmemkv_start_failure_callback_t* callback);
void pmemkv_stop(pmemkv_engine* kv);

void pmemkv_all(pmemkv_engine* kv, void* context, pmemkv_all_callback_t* c);
void pmemkv_all_above(pmemkv_engine* kv, void* context, int32_t kb, const char* k, pmemkv_all_callback_t* c);
void pmemkv_all_below(pmemkv_engine* kv, void* context, int32_t kb, const char* k, pmemkv_all_callback_t* c);
void pmemkv_all_between(pmemkv_engine* kv, void* context, int32_t kb1, const char* k1,
                          int32_t kb2, const char* k2, pmemkv_all_callback_t* c);

int64_t pmemkv_count(pmemkv_engine* kv);
int64_t pmemkv_count_above(pmemkv_engine* kv, int32_t kb, const char* k);
int64_t pmemkv_count_below(pmemkv_engine* kv, int32_t kb, const char* k);
int64_t pmemkv_count_between(pmemkv_engine* kv, int32_t kb1, const char* k1, int32_t kb2, const char* k2);

void pmemkv_each(pmemkv_engine* kv, void* context, pmemkv_each_callback_t* c);
void pmemkv_each_above(pmemkv_engine* kv, void* context, int32_t kb, const char* k, pmemkv_each_callback_t* c);
void pmemkv_each_below(pmemkv_engine* kv, void* context, int32_t kb, const char* k, pmemkv_each_callback_t* c);
void pmemkv_each_between(pmemkv_engine* kv, void* context, int32_t kb1, const char* k1,
                           int32_t kb2, const char* k2, pmemkv_each_callback_t* c);

int8_t pmemkv_exists(pmemkv_engine* kv, int32_t kb, const char* k);
void pmemkv_get(pmemkv_engine* kv, void* context, int32_t kb, const char* k, pmemkv_get_callback_t* c);
int8_t pmemkv_get_copy(pmemkv_engine* kv, int32_t kb, const char* k, int32_t maxvaluebytes, char* value);
int8_t pmemkv_put(pmemkv_engine* kv, int32_t kb, const char* k, int32_t vb, const char* v);
int8_t pmemkv_remove(pmemkv_engine* kv, int32_t kb, const char* k);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* LIBPMEMKV_H */