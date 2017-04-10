/*
 * Copyright 2017, Intel Corporation
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

#include "pmemkv.h"
#include "gtest/gtest.h"

using namespace pmemkv;

const std::string PATH = "/dev/shm/pmemkv";
const std::string PATH_CACHED = "/tmp/pmemkv";
const size_t SIZE = ((size_t) (1024 * 1024 * 1104));

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class KVEmptyTest : public testing::Test {
  public:
    KVEmptyTest() {
        std::remove(PATH.c_str());
    }
};

class KVTest : public testing::Test {
  public:
    KVTreeAnalysis analysis;
    KVTree* kv;

    KVTest() {
        std::remove(PATH.c_str());
        Open();
    }

    ~KVTest() { delete kv; }

    void Analyze() {
        analysis = {};
        kv->Analyze(analysis);
        ASSERT_TRUE(analysis.path == PATH);
    }

    void Reopen() {
        delete kv;
        Open();
    }

  private:
    void Open() {
        kv = new KVTree(PATH, SIZE);
    }
};

// =============================================================================================
// TEST EMPTY TREE
// =============================================================================================

TEST_F(KVEmptyTest, CreateInstanceTest) {
    KVTree* kv = new KVTree(PATH, PMEMOBJ_MIN_POOL);
    KVTreeAnalysis analysis = {};
    kv->Analyze(analysis);
    ASSERT_EQ(analysis.size, PMEMOBJ_MIN_POOL);
    delete kv;
}

TEST_F(KVEmptyTest, CreateInstanceFromExistingTest) {
    KVTree* kv = new KVTree(PATH, PMEMOBJ_MIN_POOL * 2);
    delete kv;
    kv = new KVTree(PATH, PMEMOBJ_MIN_POOL);
    KVTreeAnalysis analysis = {};
    kv->Analyze(analysis);
    ASSERT_EQ(analysis.size, PMEMOBJ_MIN_POOL * 2);
    delete kv;
}

TEST_F(KVEmptyTest, FailsToCreateInstanceWithInvalidPath) {
    try {
        new KVTree("/tmp/123/234/345/456/567/678/nope.nope", PMEMOBJ_MIN_POOL);
        FAIL();
    } catch (...) {
        // do nothing, expected to happen
    }
}

TEST_F(KVEmptyTest, FailsToCreateInstanceWithHugeSize) {
    try {
        new KVTree(PATH, 9223372036854775807);   // 9.22 exabytes
        FAIL();
    } catch (...) {
        // do nothing, expected to happen
    }
}

TEST_F(KVEmptyTest, FailsToCreateInstanceWithTinySize) {
    try {
        new KVTree(PATH, PMEMOBJ_MIN_POOL - 1);  // too small
        FAIL();
    } catch (...) {
        // do nothing, expected to happen
    }
}

TEST_F(KVEmptyTest, SizeofTest) {
    // persistent types
    ASSERT_EQ(sizeof(KVRoot), 16);
    ASSERT_EQ(sizeof(KVSlot), 32);
    ASSERT_EQ(sizeof(KVLeaf), 1552);

    // volatile types
    ASSERT_EQ(sizeof(KVInnerNode), 232);
    ASSERT_EQ(sizeof(KVLeafNode), 1624);
}

// =============================================================================================
// TEST SINGLE-LEAF TREE
// =============================================================================================

TEST_F(KVTest, EmptyKeyTest) {                                      // todo correct behavior?
    ASSERT_TRUE(kv->Put("", "blah") == OK);
    std::string value;
    ASSERT_TRUE(kv->Get("", &value) == OK && value == "blah");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, EmptyValueTest) {                                    // todo correct behavior?
    ASSERT_TRUE(kv->Put("key1", "") == OK);
    std::string value;
    ASSERT_TRUE(kv->Get("key1", &value) == OK && value == "");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetAppendToExternalValueTest) {
    ASSERT_TRUE(kv->Put("key1", "cool") == OK);
    std::string value = "super";
    ASSERT_TRUE(kv->Get("key1", &value) == OK && value == "supercool");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetHeadlessTest) {
    std::string value;
    ASSERT_TRUE(kv->Get("waldo", &value) == NOT_FOUND);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 0);
}

TEST_F(KVTest, GetMultipleTest) {
    ASSERT_TRUE(kv->Put("abc", "A1") == OK);
    ASSERT_TRUE(kv->Put("def", "B2") == OK);
    ASSERT_TRUE(kv->Put("hij", "C3") == OK);
    ASSERT_TRUE(kv->Put("jkl", "D4") == OK);
    ASSERT_TRUE(kv->Put("mno", "E5") == OK);
    std::string value1;
    ASSERT_TRUE(kv->Get("abc", &value1) == OK && value1 == "A1");
    std::string value2;
    ASSERT_TRUE(kv->Get("def", &value2) == OK && value2 == "B2");
    std::string value3;
    ASSERT_TRUE(kv->Get("hij", &value3) == OK && value3 == "C3");
    std::string value4;
    ASSERT_TRUE(kv->Get("jkl", &value4) == OK && value4 == "D4");
    std::string value5;
    ASSERT_TRUE(kv->Get("mno", &value5) == OK && value5 == "E5");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetMultiple2Test) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Put("key2", "value2") == OK);
    ASSERT_TRUE(kv->Put("key3", "value3") == OK);
    ASSERT_TRUE(kv->Remove("key2") == OK);
    ASSERT_TRUE(kv->Put("key3", "VALUE3") == OK);
    std::string value1;
    ASSERT_TRUE(kv->Get("key1", &value1) == OK && value1 == "value1");
    std::string value2;
    ASSERT_TRUE(kv->Get("key2", &value2) == NOT_FOUND);
    std::string value3;
    ASSERT_TRUE(kv->Get("key3", &value3) == OK && value3 == "VALUE3");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetNonexistentTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    std::string value;
    ASSERT_TRUE(kv->Get("waldo", &value) == NOT_FOUND);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetListTest) {
    ASSERT_TRUE(kv->Put("tmpkey", "tmpvalue1") == OK);
    ASSERT_TRUE(kv->Put("tmpkey2", "tmpvalue2") == OK);
    auto values = std::vector<std::string>();
    auto keys = std::vector<std::string>();
    keys.push_back("tmpkey");
    keys.push_back("tmpkey2");
    keys.push_back("tmpkey3");
    keys.push_back("tmpkey");
    std::vector<KVStatus> status = kv->GetList(keys, &values);
    ASSERT_TRUE(status.size() == 4);
    ASSERT_TRUE(values.size() == 4);
    ASSERT_TRUE(status.at(0) == OK && values.at(0) == "tmpvalue1");
    ASSERT_TRUE(status.at(1) == OK && values.at(1) == "tmpvalue2");
    ASSERT_TRUE(status.at(2) == NOT_FOUND && values.at(2) == "");
    ASSERT_TRUE(status.at(3) == OK && values.at(3) == "tmpvalue1");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, PutTest) {
    std::string value;
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Get("key1", &value) == OK && value == "value1");

    std::string new_value;
    ASSERT_TRUE(kv->Put("key1", "VALUE1") == OK);           // same size
    ASSERT_TRUE(kv->Get("key1", &new_value) == OK && new_value == "VALUE1");

    std::string new_value2;
    ASSERT_TRUE(kv->Put("key1", "new_value") == OK);        // longer size
    ASSERT_TRUE(kv->Get("key1", &new_value2) == OK && new_value2 == "new_value");

    std::string new_value3;
    ASSERT_TRUE(kv->Put("key1", "?") == OK);                // shorter size
    ASSERT_TRUE(kv->Get("key1", &new_value3) == OK && new_value3 == "?");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, PutKeysOfDifferentSizesTest) {
    std::string value;
    ASSERT_TRUE(kv->Put("123456789ABCDE", "A") == OK);
    ASSERT_TRUE(kv->Get("123456789ABCDE", &value) == OK && value == "A");

    std::string value2;
    ASSERT_TRUE(kv->Put("123456789ABCDEF", "B") == OK);
    ASSERT_TRUE(kv->Get("123456789ABCDEF", &value2) == OK && value2 == "B");

    std::string value3;
    ASSERT_TRUE(kv->Put("12345678ABCDEFG", "C") == OK);
    ASSERT_TRUE(kv->Get("12345678ABCDEFG", &value3) == OK && value3 == "C");

    std::string value4;
    ASSERT_TRUE(kv->Put("123456789", "D") == OK);
    ASSERT_TRUE(kv->Get("123456789", &value4) == OK && value4 == "D");

    std::string value5;
    ASSERT_TRUE(kv->Put("123456789ABCDEFGHI", "E") == OK);
    ASSERT_TRUE(kv->Get("123456789ABCDEFGHI", &value5) == OK && value5 == "E");
}

TEST_F(KVTest, PutValuesOfDifferentSizesTest) {
    std::string value;
    ASSERT_TRUE(kv->Put("A", "123456789ABCDE") == OK);
    ASSERT_TRUE(kv->Get("A", &value) == OK && value == "123456789ABCDE");

    std::string value2;
    ASSERT_TRUE(kv->Put("B", "123456789ABCDEF") == OK);
    ASSERT_TRUE(kv->Get("B", &value2) == OK && value2 == "123456789ABCDEF");

    std::string value3;
    ASSERT_TRUE(kv->Put("C", "12345678ABCDEFG") == OK);
    ASSERT_TRUE(kv->Get("C", &value3) == OK && value3 == "12345678ABCDEFG");

    std::string value4;
    ASSERT_TRUE(kv->Put("D", "123456789") == OK);
    ASSERT_TRUE(kv->Get("D", &value4) == OK && value4 == "123456789");

    std::string value5;
    ASSERT_TRUE(kv->Put("E", "123456789ABCDEFGHI") == OK);
    ASSERT_TRUE(kv->Get("E", &value5) == OK && value5 == "123456789ABCDEFGHI");
}

TEST_F(KVTest, RemoveAllTest) {
    ASSERT_TRUE(kv->Put("tmpkey", "tmpvalue1") == OK);
    ASSERT_TRUE(kv->Remove("tmpkey") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveAndInsertTest) {
    ASSERT_TRUE(kv->Put("tmpkey", "tmpvalue1") == OK);
    ASSERT_TRUE(kv->Remove("tmpkey") == OK);
    ASSERT_TRUE(kv->Put("tmpkey1", "tmpvalue1") == OK);
    std::string value;
    ASSERT_TRUE(kv->Get("tmpkey1", &value) == OK && value == "tmpvalue1");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveExistingTest) {
    ASSERT_TRUE(kv->Put("tmpkey1", "tmpvalue1") == OK);
    ASSERT_TRUE(kv->Put("tmpkey2", "tmpvalue2") == OK);
    ASSERT_TRUE(kv->Remove("tmpkey1") == OK);
    ASSERT_TRUE(kv->Remove("tmpkey1") == OK); // ok to remove twice
    std::string value;
    ASSERT_TRUE(kv->Get("tmpkey1", &value) == NOT_FOUND);
    ASSERT_TRUE(kv->Get("tmpkey2", &value) == OK && value == "tmpvalue2");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveHeadlessTest) {
    ASSERT_TRUE(kv->Remove("nada") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 0);
}

TEST_F(KVTest, RemoveNonexistentTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Remove("nada") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

// =============================================================================================
// TEST RECOVERY OF SINGLE-LEAF TREE
// =============================================================================================

TEST_F(KVTest, GetHeadlessAfterRecoveryTest) {
    Reopen();
    std::string value;
    ASSERT_TRUE(kv->Get("waldo", &value) == NOT_FOUND);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 0);
}

TEST_F(KVTest, GetMultipleAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("abc", "A1") == OK);
    ASSERT_TRUE(kv->Put("def", "B2") == OK);
    ASSERT_TRUE(kv->Put("hij", "C3") == OK);
    Reopen();
    ASSERT_TRUE(kv->Put("jkl", "D4") == OK);
    ASSERT_TRUE(kv->Put("mno", "E5") == OK);
    std::string value1;
    ASSERT_TRUE(kv->Get("abc", &value1) == OK && value1 == "A1");
    std::string value2;
    ASSERT_TRUE(kv->Get("def", &value2) == OK && value2 == "B2");
    std::string value3;
    ASSERT_TRUE(kv->Get("hij", &value3) == OK && value3 == "C3");
    std::string value4;
    ASSERT_TRUE(kv->Get("jkl", &value4) == OK && value4 == "D4");
    std::string value5;
    ASSERT_TRUE(kv->Get("mno", &value5) == OK && value5 == "E5");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetMultiple2AfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Put("key2", "value2") == OK);
    ASSERT_TRUE(kv->Put("key3", "value3") == OK);
    ASSERT_TRUE(kv->Remove("key2") == OK);
    ASSERT_TRUE(kv->Put("key3", "VALUE3") == OK);
    Reopen();
    std::string value1;
    ASSERT_TRUE(kv->Get("key1", &value1) == OK && value1 == "value1");
    std::string value2;
    ASSERT_TRUE(kv->Get("key2", &value2) == NOT_FOUND);
    std::string value3;
    ASSERT_TRUE(kv->Get("key3", &value3) == OK && value3 == "VALUE3");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, GetNonexistentAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    Reopen();
    std::string value;
    ASSERT_TRUE(kv->Get("waldo", &value) == NOT_FOUND);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, PutAfterRecoveryTest) {
    std::string value;
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Get("key1", &value) == OK && value == "value1");

    std::string new_value;
    ASSERT_TRUE(kv->Put("key1", "VALUE1") == OK);           // same size
    ASSERT_TRUE(kv->Get("key1", &new_value) == OK && new_value == "VALUE1");
    Reopen();

    std::string new_value2;
    ASSERT_TRUE(kv->Put("key1", "new_value") == OK);        // longer size
    ASSERT_TRUE(kv->Get("key1", &new_value2) == OK && new_value2 == "new_value");

    std::string new_value3;
    ASSERT_TRUE(kv->Put("key1", "?") == OK);                // shorter size
    ASSERT_TRUE(kv->Get("key1", &new_value3) == OK && new_value3 == "?");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveAllAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("tmpkey", "tmpvalue1") == OK);
    Reopen();
    ASSERT_TRUE(kv->Remove("tmpkey") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveAndInsertAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("tmpkey", "tmpvalue1") == OK);
    Reopen();
    ASSERT_TRUE(kv->Remove("tmpkey") == OK);
    ASSERT_TRUE(kv->Put("tmpkey1", "tmpvalue1") == OK);
    std::string value;
    ASSERT_TRUE(kv->Get("tmpkey1", &value) == OK && value == "tmpvalue1");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveExistingAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("tmpkey1", "tmpvalue1") == OK);
    ASSERT_TRUE(kv->Put("tmpkey2", "tmpvalue2") == OK);
    ASSERT_TRUE(kv->Remove("tmpkey1") == OK);
    Reopen();
    ASSERT_TRUE(kv->Remove("tmpkey1") == OK); // ok to remove twice
    std::string value;
    ASSERT_TRUE(kv->Get("tmpkey1", &value) == NOT_FOUND);
    ASSERT_TRUE(kv->Get("tmpkey2", &value) == OK && value == "tmpvalue2");
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, RemoveHeadlessAfterRecoveryTest) {
    Reopen();
    ASSERT_TRUE(kv->Remove("nada") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 0);
}

TEST_F(KVTest, RemoveNonexistentAfterRecoveryTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    Reopen();
    ASSERT_TRUE(kv->Remove("nada") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

TEST_F(KVTest, UsePreallocAfterSingleLeafRecoveryTest) {
    ASSERT_TRUE(kv->Put("key1", "value1") == OK);
    ASSERT_TRUE(kv->Remove("key1") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);

    Reopen();
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 1);
    ASSERT_EQ(analysis.leaf_total, 1);

    ASSERT_TRUE(kv->Put("key2", "value2") == OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 1);
}

// =============================================================================================
// TEST TREE WITH SINGLE INNER NODE
// =============================================================================================

const int SINGLE_INNER_LIMIT = LEAF_KEYS * (INNER_KEYS - 1);

TEST_F(KVTest, SingleInnerNodeAscendingTest) {
    for (int i = 10000; i <= (10000 + SINGLE_INNER_LIMIT); i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    for (int i = 10000; i <= (10000 + SINGLE_INNER_LIMIT); i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

TEST_F(KVTest, SingleInnerNodeAscendingTest2) {
    for (int i = 1; i <= SINGLE_INNER_LIMIT; i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    for (int i = 1; i <= SINGLE_INNER_LIMIT; i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

TEST_F(KVTest, SingleInnerNodeDescendingTest) {
    for (int i = (10000 + SINGLE_INNER_LIMIT); i >= 10000; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    for (int i = (10000 + SINGLE_INNER_LIMIT); i >= 10000; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 6);
}

TEST_F(KVTest, SingleInnerNodeDescendingTest2) {
    for (int i = SINGLE_INNER_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    for (int i = SINGLE_INNER_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

// =============================================================================================
// TEST RECOVERY OF TREE WITH SINGLE INNER NODE
// =============================================================================================

TEST_F(KVTest, SingleInnerNodeAscendingAfterRecoveryTest) {
    for (int i = 10000; i <= (10000 + SINGLE_INNER_LIMIT); i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
    }
    Reopen();
    for (int i = 10000; i <= (10000 + SINGLE_INNER_LIMIT); i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

TEST_F(KVTest, SingleInnerNodeAscendingAfterRecoveryTest2) {
    for (int i = 1; i <= SINGLE_INNER_LIMIT; i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
    }
    Reopen();
    for (int i = 1; i <= SINGLE_INNER_LIMIT; i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

TEST_F(KVTest, SingleInnerNodeDescendingAfterRecoveryTest) {
    for (int i = (10000 + SINGLE_INNER_LIMIT); i >= 10000; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
    }
    Reopen();
    for (int i = (10000 + SINGLE_INNER_LIMIT); i >= 10000; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 6);
}

TEST_F(KVTest, SingleInnerNodeDescendingAfterRecoveryTest2) {
    for (int i = SINGLE_INNER_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, istr) == OK);
    }
    Reopen();
    for (int i = SINGLE_INNER_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == istr);
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 5);
}

TEST_F(KVTest, UsePreallocAfterMultipleLeafRecoveryTest) {
    for (int i = 1; i <= LEAF_KEYS + 1; i++) ASSERT_EQ(kv->Put(std::to_string(i), "!"), OK);
    Reopen();
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 2);

    for (int i = 1; i <= LEAF_KEYS; i++) ASSERT_EQ(kv->Remove(std::to_string(i)), OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 2);
    Reopen();
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 1);
    ASSERT_EQ(analysis.leaf_total, 2);

    ASSERT_EQ(kv->Remove(std::to_string(LEAF_KEYS + 1)), OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 2);
    ASSERT_EQ(analysis.leaf_prealloc, 1);
    ASSERT_EQ(analysis.leaf_total, 2);
    Reopen();
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 2);
    ASSERT_EQ(analysis.leaf_prealloc, 2);
    ASSERT_EQ(analysis.leaf_total, 2);

    for (int i = 1; i <= LEAF_KEYS; i++) ASSERT_EQ(kv->Put(std::to_string(i), "!"), OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 1);
    ASSERT_EQ(analysis.leaf_prealloc, 1);
    ASSERT_EQ(analysis.leaf_total, 2);
    ASSERT_EQ(kv->Put(std::to_string(LEAF_KEYS + 1), "!"), OK);
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 2);
}

// =============================================================================================
// TEST LARGE TREE
// =============================================================================================

const int LARGE_LIMIT = 6012299;

TEST_F(KVTest, LargeAscendingTest) {
    for (int i = 1; i <= LARGE_LIMIT; i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, (istr + "!")) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == (istr + "!"));
    }
    for (int i = 1; i <= LARGE_LIMIT; i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == (istr + "!"));
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 229126);
}

TEST_F(KVTest, LargeDescendingTest) {
    for (int i = LARGE_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, ("ABC" + istr)) == OK);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == ("ABC" + istr));
    }
    for (int i = LARGE_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == ("ABC" + istr));
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 225461);
}

// =============================================================================================
// TEST RECOVERY OF LARGE TREE
// =============================================================================================

TEST_F(KVTest, LargeAscendingAfterRecoveryTest) {
    for (int i = 1; i <= LARGE_LIMIT; i++) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, (istr + "!")) == OK);
    }
    Reopen();
    for (int i = 1; i <= LARGE_LIMIT; i++) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == (istr + "!"));
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 229126);
}

TEST_F(KVTest, LargeDescendingAfterRecoveryTest) {
    for (int i = LARGE_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        ASSERT_TRUE(kv->Put(istr, ("ABC" + istr)) == OK);
    }
    Reopen();
    for (int i = LARGE_LIMIT; i >= 1; i--) {
        std::string istr = std::to_string(i);
        std::string value;
        ASSERT_TRUE(kv->Get(istr, &value) == OK && value == ("ABC" + istr));
    }
    Analyze();
    ASSERT_EQ(analysis.leaf_empty, 0);
    ASSERT_EQ(analysis.leaf_prealloc, 0);
    ASSERT_EQ(analysis.leaf_total, 225461);
}

// =============================================================================================
// TEST RUNNING OUT OF SPACE
// =============================================================================================

class KVFullTest : public testing::Test {
  public:
    KVTree* kv;

    KVFullTest() {
        std::remove(PATH.c_str());
        Open();
    }

    ~KVFullTest() { delete kv; }

    void Reopen() {
        delete kv;
        kv = new KVTree(PATH, SIZE);
    }

    void Validate() {
        for (int i = 1; i <= LARGE_LIMIT; i++) {
            std::string istr = std::to_string(i);
            std::string value;
            ASSERT_TRUE(kv->Get(istr, &value) == OK && value == (istr + "!"));
        }
    }

  private:
    void Open() {
        if (access(PATH_CACHED.c_str(), F_OK) == 0) {
            ASSERT_TRUE(std::system(("cp -f " + PATH_CACHED + " " + PATH).c_str()) == 0);
        } else {
            std::cout << "!!! creating cached copy at " << PATH_CACHED << "\n";
            KVTree* kvt = new KVTree(PATH, SIZE);
            for (int i = 1; i <= LARGE_LIMIT; i++) {
                std::string istr = std::to_string(i);
                ASSERT_TRUE(kvt->Put(istr, (istr + "!")) == OK);
            }
            delete kvt;
            ASSERT_TRUE(std::system(("cp -f " + PATH + " " + PATH_CACHED).c_str()) == 0);
        }
        kv = new KVTree(PATH, SIZE);
    }
};

const string LONGSTR = "123456789A123456789A123456789A123456789A123456789A123456789A123456789A";

TEST_F(KVFullTest, OutOfSpace1Test) {
    ASSERT_TRUE(kv->Put("100", "?") == FAILED);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace2aTest) {
    ASSERT_TRUE(kv->Remove("100") == OK);
    ASSERT_TRUE(kv->Put("100", LONGSTR) == FAILED);
    ASSERT_TRUE(kv->Put("100", "100!") == OK);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace2bTest) {
    ASSERT_TRUE(kv->Remove("100") == OK);
    ASSERT_TRUE(kv->Put("100", "100!") == OK);
    ASSERT_TRUE(kv->Put("100", LONGSTR) == FAILED);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace3aTest) {
    ASSERT_TRUE(kv->Put("100", LONGSTR) == FAILED);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace3bTest) {
    for (int i = 0; i <= 99999; i++) {
        ASSERT_TRUE(kv->Put("123456", LONGSTR) == FAILED);
    }
    ASSERT_TRUE(kv->Remove("4567") == OK);
    ASSERT_TRUE(kv->Put("4567", "4567!") == OK);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace4aTest) {
    ASSERT_TRUE(kv->Put(std::to_string(LARGE_LIMIT + 1), "1") == FAILED);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace4bTest) {
    for (int i = 0; i <= 99999; i++) {
        ASSERT_TRUE(kv->Put(std::to_string(LARGE_LIMIT + 1), "1") == FAILED);
    }
    ASSERT_TRUE(kv->Remove("98765") == OK);
    ASSERT_TRUE(kv->Put("98765", "98765!") == OK);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace5aTest) {
    ASSERT_TRUE(kv->Put(LONGSTR, "1") == FAILED);
    ASSERT_TRUE(kv->Put(LONGSTR, LONGSTR) == FAILED);
    Validate();
}

TEST_F(KVFullTest, OutOfSpace5bTest) {
    for (int i = 0; i <= 99999; i++) {
        ASSERT_TRUE(kv->Put(LONGSTR, "1") == FAILED);
        ASSERT_TRUE(kv->Put(LONGSTR, LONGSTR) == FAILED);
    }
    ASSERT_TRUE(kv->Remove("34567") == OK);
    ASSERT_TRUE(kv->Put("34567", "34567!") == OK);
    Validate();
}

TEST_F(KVFullTest, RepeatedRecoveryTest) {
    for (int i = 1; i <= 100; i++) Reopen();
    Validate();
}
