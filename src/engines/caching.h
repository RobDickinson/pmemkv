/*
 * Copyright 2017-2018, Intel Corporation
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

#pragma once

#include "../pmemkv.h"
#include <string>
#include <cstring>
#include <ctime>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

namespace pmemkv {
namespace caching {

const string ENGINE = "caching";
static int ttl;

class CachingEngine : public KVEngine {
  public:
    CachingEngine(const string& config);
    ~CachingEngine();
    string Engine() final { return ENGINE; }
    using KVEngine::All;
    void All(void* context, KVAllCallback* callback) final;
    int64_t Count() final;
    using KVEngine::Each;
    void Each(void* context, KVEachCallback* callback) final;
    KVStatus Exists(const string& key) final;
    using KVEngine::Get;
    void Get(void* context, const string& key, KVGetCallback* callback) final;
    KVStatus Put(const string& key, const string& value) final;
    KVStatus Remove(const string& key) final;
    bool readConfig(const string& config);
    bool getFromRemoteRedis(const std::string &key, std::string &value);
    bool getFromRemoteMemcached(const std::string &key, std::string &value);
    bool getKey(const string& key, string &valueField, bool api_flag);
  private:
    std::string subEngineConfig;
    std::string subEngine;
    std::string remoteType;
    std::string remoteUser;
    std::string remotePasswd;
    std::string remoteUrl;
    std::string host;
    pmemkv::KVEngine *basePtr;
    unsigned long int port;
    int attempts;
};

extern "C" {
  time_t convertTimeToEpoch( const char* theTime,const char* format = "%Y%m%d%H%M%S");
  string getTimeStamp(time_t epochTime, const char* format = "%Y%m%d%H%M%S");
  bool valueFieldConversion(string dateValue);
}
} // namespace caching
} // namespace pmemkv
