/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/transport/TZlibTransport.h>

#include <gtest/gtest.h>

using namespace std;
using namespace apache::thrift::transport;

/*
 * Utility code
 */

namespace {

class SizeGenerator {
 public:
  virtual ~SizeGenerator() {}
  virtual unsigned int getSize() = 0;
};

class ConstantSizeGenerator : public SizeGenerator {
 public:
  explicit ConstantSizeGenerator(unsigned int value) : value_(value) {}
  unsigned int getSize() override { return value_; }

 private:
  unsigned int value_;
};

class LogNormalSizeGenerator : public SizeGenerator {
 public:
  LogNormalSizeGenerator(double mean, double std_dev) : dist_(mean, std_dev) {}

  unsigned int getSize() override {
    // Loop until we get a size of 1 or more
    while (true) {
      auto value = int(log(dist_(rng_)));
      if (value >= 1) {
        return value;
      }
    }
  }

 private:
  mt19937 rng_;
  lognormal_distribution<double> dist_;
};

vector<uint8_t> gen_uniform_buffer(uint32_t buf_len, uint8_t c) {
  return vector<uint8_t>(buf_len, c);
}

vector<uint8_t> gen_compressible_buffer(uint32_t buf_len) {
  vector<uint8_t> buf(buf_len, 0);

  // Generate small runs of alternately increasing and decreasing bytes
  mt19937 rng;
  uniform_int_distribution<uint32_t> run_length_distribution(1, 64);
  uniform_int_distribution<uint8_t> byte_distribution;

  uint32_t idx = 0;
  int8_t step = 1;
  while (idx < buf_len) {
    uint32_t run_length = run_length_distribution(rng);
    if (idx + run_length > buf_len) {
      run_length = buf_len - idx;
    }

    uint8_t byte = byte_distribution(rng);
    for (uint32_t n = 0; n < run_length; ++n) {
      buf[idx] = byte;
      ++idx;
      byte += step;
    }

    step *= -1;
  }

  return buf;
}

vector<uint8_t> gen_random_buffer(uint32_t buf_len) {
  vector<uint8_t> buf(buf_len, 0);

  mt19937 rng;
  uniform_int_distribution<uint8_t> distribution;

  for (uint32_t n = 0; n < buf_len; ++n) {
    buf[n] = distribution(rng);
  }

  return buf;
}

constexpr uint32_t kBufLen = 1024 * 32;
const auto kExampleUniformBuffer = gen_uniform_buffer(kBufLen, 'a');
const auto kExampleCompressibleBuffer = gen_compressible_buffer(kBufLen);
const auto kExampleRandomBuffer = gen_random_buffer(kBufLen);

/*
 * Test functions
 */

void test_write_then_read(const vector<uint8_t>& buf) {
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  zlib_trans->write(buf.data(), buf.size());
  zlib_trans->finish();

  vector<uint8_t> mirror(buf.size(), 0);
  uint32_t got = zlib_trans->readAll(mirror.data(), buf.size());
  ASSERT_EQ(got, buf.size());
  EXPECT_EQ(memcmp(mirror.data(), buf.data(), buf.size()), 0);
  zlib_trans->verifyChecksum();
}

void test_separate_checksum(const vector<uint8_t>& buf) {
  // This one is tricky.  I separate the last byte of the stream out
  // into a separate crbuf_.  The last byte is part of the checksum,
  // so the entire read goes fine, but when I go to verify the checksum
  // it isn't there.  The original implementation complained that
  // the stream was not complete.  I'm about to go fix that.
  // It worked.  Awesome.
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  zlib_trans->write(buf.data(), buf.size());
  zlib_trans->finish();
  string tmp_buf;
  membuf->appendBufferToString(tmp_buf);
  auto urbuf_size = TZlibTransport::DEFAULT_URBUF_SIZE;
  zlib_trans =
      make_shared<TZlibTransport>(membuf, urbuf_size, tmp_buf.length() - 1);

  vector<uint8_t> mirror(buf.size(), 0);
  uint32_t got = zlib_trans->readAll(mirror.data(), buf.size());
  ASSERT_EQ(got, buf.size());
  EXPECT_EQ(memcmp(mirror.data(), buf.data(), buf.size()), 0);
  zlib_trans->verifyChecksum();
}

void test_incomplete_checksum(const vector<uint8_t>& buf) {
  // Make sure we still get that "not complete" error if
  // it really isn't complete.
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  zlib_trans->write(buf.data(), buf.size());
  zlib_trans->finish();
  string tmp_buf;
  membuf->appendBufferToString(tmp_buf);
  tmp_buf.erase(tmp_buf.length() - 1);
  membuf->resetBuffer(
      const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(tmp_buf.data())),
      tmp_buf.length());

  vector<uint8_t> mirror(buf.size(), 0);
  uint32_t got = zlib_trans->readAll(mirror.data(), buf.size());
  ASSERT_EQ(got, buf.size());
  EXPECT_EQ(memcmp(mirror.data(), buf.data(), buf.size()), 0);
  try {
    zlib_trans->verifyChecksum();
    ADD_FAILURE() << "verifyChecksum() did not report an error";
  } catch (TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::CORRUPTED_DATA);
  }
}

void test_read_write_mix(
    const vector<uint8_t>& buf,
    const shared_ptr<SizeGenerator>& write_gen,
    const shared_ptr<SizeGenerator>& read_gen) {
  // Try it with a mix of read/write sizes.
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  unsigned int tot;

  tot = 0;
  while (tot < buf.size()) {
    uint32_t write_len = write_gen->getSize();
    if (tot + write_len > buf.size()) {
      write_len = buf.size() - tot;
    }
    zlib_trans->write(buf.data() + tot, write_len);
    tot += write_len;
  }

  zlib_trans->finish();

  tot = 0;
  vector<uint8_t> mirror(buf.size(), 0);
  while (tot < buf.size()) {
    uint32_t read_len = read_gen->getSize();
    uint32_t expected_read_len = read_len;
    if (tot + read_len > buf.size()) {
      expected_read_len = buf.size() - tot;
    }
    uint32_t got = zlib_trans->read(mirror.data() + tot, read_len);
    ASSERT_LE(got, expected_read_len);
    ASSERT_NE(got, 0);
    tot += got;
  }

  EXPECT_EQ(memcmp(mirror.data(), buf.data(), buf.size()), 0);
  zlib_trans->verifyChecksum();
}

void test_invalid_checksum(const vector<uint8_t>& buf) {
  // Verify checksum checking.
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  zlib_trans->write(buf.data(), buf.size());
  zlib_trans->finish();
  string tmp_buf;
  membuf->appendBufferToString(tmp_buf);
  // Modify a byte at the end of the buffer (part of the checksum).
  // On rare occasions, modifying a byte in the middle of the buffer
  // isn't caught by the checksum.
  //
  // (This happens especially often for the uniform buffer.  The
  // re-inflated data is correct, however.  I suspect in this case that
  // we're more likely to modify bytes that are part of zlib metadata
  // instead of the actual compressed data.)
  //
  // I've also seen some failure scenarios where a checksum failure isn't
  // reported, but zlib keeps trying to decode past the end of the data.
  // (When this occurs, verifyChecksum() throws an exception indicating
  // that the end of the data hasn't been reached.)  I haven't seen this
  // error when only modifying checksum bytes.
  int index = tmp_buf.size() - 1;
  tmp_buf[index]++;
  membuf->resetBuffer(
      const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(tmp_buf.data())),
      tmp_buf.length());

  vector<uint8_t> mirror(buf.size(), 0);
  try {
    zlib_trans->readAll(mirror.data(), buf.size());
    zlib_trans->verifyChecksum();
    ADD_FAILURE() << "verifyChecksum() did not report an error";
  } catch (TZlibTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::INTERNAL_ERROR);
  }
}

void test_write_after_flush(const vector<uint8_t>& buf) {
  // write some data
  auto membuf = make_shared<TMemoryBuffer>();
  auto zlib_trans = make_shared<TZlibTransport>(membuf);
  zlib_trans->write(buf.data(), buf.size());

  // call finish()
  zlib_trans->finish();

  // make sure write() throws an error
  try {
    uint8_t write_buf[] = "a";
    zlib_trans->write(write_buf, 1);
    ADD_FAILURE() << "write() after finish() did not raise an exception";
  } catch (TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::BAD_ARGS);
  }

  // make sure flush() throws an error
  try {
    zlib_trans->flush();
    ADD_FAILURE() << "flush() after finish() did not raise an exception";
  } catch (TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::BAD_ARGS);
  }

  // make sure finish() throws an error
  try {
    zlib_trans->finish();
    ADD_FAILURE() << "finish() after finish() did not raise an exception";
  } catch (TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::BAD_ARGS);
  }
}

void test_no_write() {
  // Verify that no data is written to the underlying transport if we
  // never write data to the TZlibTransport.
  auto membuf = make_shared<TMemoryBuffer>();
  {
    // Create a TZlibTransport object, and immediately destroy it
    // when it goes out of scope.
    TZlibTransport w_zlib_trans(membuf);
  }

  EXPECT_EQ(membuf->available_read(), 0);
}

/*
 * Initialization
 */

class ZlibTest : public testing::Test {
 public:
  shared_ptr<SizeGenerator> size32k{
      make_shared<ConstantSizeGenerator>(1 << 15)};
  shared_ptr<SizeGenerator> sizeLognormal{
      make_shared<LogNormalSizeGenerator>(20, 30)};
  shared_ptr<SizeGenerator> writeSizeGen{
      make_shared<LogNormalSizeGenerator>(20, 30)};
  shared_ptr<SizeGenerator> readSizeGen{
      make_shared<LogNormalSizeGenerator>(20, 30)};
};

} // namespace

#define ADD_TEST_CASE(name, function, ...) \
  TEST_F(ZlibTest, name##_##function) {    \
    test_##function(__VA_ARGS__);          \
  }

#define ADD_TESTS(name, buf)                                               \
  ADD_TEST_CASE(name, write_then_read, buf)                                \
  ADD_TEST_CASE(name, separate_checksum, buf)                              \
  ADD_TEST_CASE(name, incomplete_checksum, buf)                            \
  ADD_TEST_CASE(name, invalid_checksum, buf)                               \
  ADD_TEST_CASE(name, write_after_flush, buf)                              \
  ADD_TEST_CASE(name##_constant, read_write_mix, buf, size32k, size32k)    \
  ADD_TEST_CASE(                                                           \
      name##_lognormal_write, read_write_mix, buf, sizeLognormal, size32k) \
  ADD_TEST_CASE(                                                           \
      name##_lognormal_read, read_write_mix, buf, size32k, sizeLognormal)  \
  ADD_TEST_CASE(                                                           \
      name##_lognormal_both,                                               \
      read_write_mix,                                                      \
      buf,                                                                 \
      sizeLognormal,                                                       \
      sizeLognormal)                                                       \
  ADD_TEST_CASE(                                                           \
      name##_lognormal_same_distribution,                                  \
      read_write_mix,                                                      \
      buf,                                                                 \
      writeSizeGen,                                                        \
      readSizeGen)

ADD_TESTS(uniform, kExampleUniformBuffer)
ADD_TESTS(compressible, kExampleCompressibleBuffer)
ADD_TESTS(random, kExampleRandomBuffer)

TEST_F(ZlibTest, test_no_write) {
  test_no_write();
}
