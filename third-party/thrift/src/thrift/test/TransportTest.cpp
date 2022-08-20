/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <csignal>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <random>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/experimental/TestUtil.h>
#include <folly/portability/Unistd.h>

#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/transport/TFDTransport.h>
#include <thrift/lib/cpp/transport/TSocket.h>
#include <thrift/lib/cpp/transport/TZlibTransport.h>

#include <folly/portability/GTest.h>

using namespace std;
using namespace folly;
using namespace apache::thrift::transport;

namespace {

DEFINE_int32(size_multiplier, 1, "");

class SizeGenerator {
 public:
  virtual ~SizeGenerator() {}
  virtual uint32_t nextSize() = 0;
  virtual string describe() const = 0;
};

class ConstantSizeGenerator : public SizeGenerator {
 public:
  /* implicit */ ConstantSizeGenerator(uint32_t value) : value_(value) {}
  uint32_t nextSize() override { return value_; }
  string describe() const override { return to<string>(value_); }

 private:
  uint32_t value_;
};

class RandomSizeGenerator : public SizeGenerator {
 public:
  RandomSizeGenerator(uint32_t min, uint32_t max) : dist_(min, max) {}

  uint32_t nextSize() override { return dist_(rng_); }

  string describe() const override {
    return sformat("rand({}, {})", getMin(), getMax());
  }

  uint32_t getMin() const { return dist_.min(); }
  uint32_t getMax() const { return dist_.max(); }

 private:
  mt19937 rng_;
  uniform_int_distribution<int> dist_;
};

/**
 * This class exists solely to make the TEST_RW() macro easier to use.
 * - it can be constructed implicitly from an integer
 * - it can contain either a ConstantSizeGenerator or a RandomSizeGenerator
 *   (TEST_RW can't take a SizeGenerator pointer or reference, since it needs
 *   to make a copy of the generator to bind it to the test function.)
 */
class GenericSizeGenerator : public SizeGenerator {
 public:
  /* implicit */ GenericSizeGenerator(uint32_t value)
      : generator_(make_shared<ConstantSizeGenerator>(value)) {}
  GenericSizeGenerator(uint32_t min, uint32_t max)
      : generator_(make_shared<RandomSizeGenerator>(min, max)) {}

  uint32_t nextSize() override { return generator_->nextSize(); }
  string describe() const override { return generator_->describe(); }

 private:
  shared_ptr<SizeGenerator> generator_;
};

/**************************************************************************
 * Classes to set up coupled transports
 **************************************************************************/

/**
 * Helper class to represent a coupled pair of transports.
 *
 * Data written to the out transport can be read from the in transport.
 *
 * This is used as the base class for the various coupled transport
 * implementations.  It shouldn't be instantiated directly.
 */
template <class Transport_>
class CoupledTransports {
 public:
  typedef Transport_ TransportType;

  CoupledTransports() : in(), out() {}

  shared_ptr<Transport_> in;
  shared_ptr<Transport_> out;

 private:
  CoupledTransports(const CoupledTransports&);
  CoupledTransports& operator=(const CoupledTransports&);
};

/**
 * Coupled TMemoryBuffers
 */
class CoupledMemoryBuffers : public CoupledTransports<TMemoryBuffer> {
 public:
  CoupledMemoryBuffers() : buf(make_shared<TMemoryBuffer>()) {
    in = buf;
    out = buf;
  }

  shared_ptr<TMemoryBuffer> buf;
};

/**
 * Helper template class for creating coupled transports that wrap
 * another transport.
 */
template <class WrapperTransport_, class InnerCoupledTransports_>
class CoupledWrapperTransportsT : public CoupledTransports<WrapperTransport_> {
 public:
  CoupledWrapperTransportsT() {
    if (inner_.in) {
      this->in = make_shared<WrapperTransport_>(inner_.in);
    }
    if (inner_.out) {
      this->out = make_shared<WrapperTransport_>(inner_.out);
    }
  }

  InnerCoupledTransports_ inner_;
};

/**
 * Coupled TBufferedTransports.
 */
template <class InnerTransport_>
class CoupledBufferedTransportsT
    : public CoupledWrapperTransportsT<TBufferedTransport, InnerTransport_> {};

typedef CoupledBufferedTransportsT<CoupledMemoryBuffers>
    CoupledBufferedTransports;

/**
 * Coupled TFramedTransports.
 */
template <class InnerTransport_>
class CoupledFramedTransportsT
    : public CoupledWrapperTransportsT<TFramedTransport, InnerTransport_> {};

typedef CoupledFramedTransportsT<CoupledMemoryBuffers> CoupledFramedTransports;

/**
 * Coupled TZlibTransports.
 */
template <class InnerTransport_>
class CoupledZlibTransportsT
    : public CoupledWrapperTransportsT<TZlibTransport, InnerTransport_> {};

typedef CoupledZlibTransportsT<CoupledMemoryBuffers> CoupledZlibTransports;

/**
 * Coupled TFDTransports.
 */
class CoupledFDTransports : public CoupledTransports<TFDTransport> {
 public:
  CoupledFDTransports() {
    int pipes[2];

    if (pipe(pipes) != 0) {
      return;
    }

    in = make_shared<TFDTransport>(pipes[0], TFDTransport::CLOSE_ON_DESTROY);
    out = make_shared<TFDTransport>(pipes[1], TFDTransport::CLOSE_ON_DESTROY);
  }
};

/**
 * Coupled TSockets
 */
class CoupledSocketTransports : public CoupledTransports<TSocket> {
 public:
  CoupledSocketTransports() {
    int sockets[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
      return;
    }

    in = make_shared<TSocket>(sockets[0]);
    out = make_shared<TSocket>(sockets[1]);
  }
};

/**
 * Wrapper around another CoupledTransports implementation that exposes the
 * transports as TTransport pointers.
 *
 * This is used since accessing a transport via a "TTransport*" exercises a
 * different code path than using the base pointer class.  As part of the
 * template code changes, most transport methods are no longer virtual.
 */
template <class CoupledTransports_>
class CoupledTTransports : public CoupledTransports<TTransport> {
 public:
  CoupledTTransports() : transports() {
    in = transports.in;
    out = transports.out;
  }

  CoupledTransports_ transports;
};

/**
 * Wrapper around another CoupledTransports implementation that exposes the
 * transports as TBufferBase pointers.
 *
 * This can only be instantiated with a transport type that is a subclass of
 * TBufferBase.
 */
template <class CoupledTransports_>
class CoupledBufferBases : public CoupledTransports<TBufferBase> {
 public:
  CoupledBufferBases() : transports() {
    in = transports.in;
    out = transports.out;
  }

  CoupledTransports_ transports;
};

/**
 * Just for templates magic.
 */
template <class T>
using IdentityT = T;

/**************************************************************************
 * Alarm handling code for use in tests that check the transport blocking
 * semantics.
 *
 * If the transport ends up blocking, we don't want to hang forever.  We use
 * SIGALRM to fire schedule signal to wake up and try to write data so the
 * transport will unblock.
 *
 * It isn't really the safest thing in the world to be mucking around with
 * complicated global data structures in a signal handler.  It should probably
 * be okay though, since we know the main thread should always be blocked in a
 * read() request when the signal handler is running.
 **************************************************************************/

struct TriggerInfo {
  TriggerInfo(
      int seconds,
      const shared_ptr<TTransport>& transport,
      uint32_t writeLength)
      : timeoutSeconds(seconds),
        transport(transport),
        writeLength(writeLength),
        next(nullptr) {}

  int timeoutSeconds;
  shared_ptr<TTransport> transport;
  uint32_t writeLength;
  TriggerInfo* next;
};

TriggerInfo* triggerInfo;
unsigned int numTriggersFired;

void set_alarm();

void alarm_handler(int /* signum */) {
  // The alarm timed out, which almost certainly means we're stuck
  // on a transport that is incorrectly blocked.
  ++numTriggersFired;

  LOG(INFO) << "Timeout alarm expired; attempting to unblock transport";
  if (triggerInfo == nullptr) {
    LOG(INFO) << "  trigger stack is empty!";
  }

  // Pop off the first TriggerInfo.
  // If there is another one, schedule an alarm for it.
  TriggerInfo* info = triggerInfo;
  triggerInfo = info->next;
  set_alarm();

  // Write some data to the transport to hopefully unblock it.
  uint8_t buf[info->writeLength];
  memset(buf, 'b', info->writeLength);
  info->transport->write(buf, info->writeLength);
  info->transport->flush();

  delete info;
}

void set_alarm() {
  if (triggerInfo == nullptr) {
    // clear any alarm
    alarm(0);
    return;
  }

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = alarm_handler;
  action.sa_flags = SA_ONESHOT;
  sigemptyset(&action.sa_mask);
  sigaction(SIGALRM, &action, nullptr);

  alarm(triggerInfo->timeoutSeconds);
}

/**
 * Add a trigger to be scheduled "seconds" seconds after the
 * last currently scheduled trigger.
 *
 * (Note that this is not "seconds" from now.  That might be more logical, but
 * would require slightly more complicated sorting, rather than just appending
 * to the end.)
 */
void add_trigger(
    unsigned int seconds,
    const shared_ptr<TTransport>& transport,
    uint32_t write_len) {
  TriggerInfo* info = new TriggerInfo(seconds, transport, write_len);

  if (triggerInfo == nullptr) {
    // This is the first trigger.
    // Set triggerInfo, and schedule the alarm
    triggerInfo = info;
    set_alarm();
  } else {
    // Add this trigger to the end of the list
    TriggerInfo* prev = triggerInfo;
    while (prev->next) {
      prev = prev->next;
    }

    prev->next = info;
  }
}

void clear_triggers() {
  TriggerInfo* info = triggerInfo;
  alarm(0);
  triggerInfo = nullptr;
  numTriggersFired = 0;

  while (info != nullptr) {
    TriggerInfo* next = info->next;
    delete info;
    info = next;
  }
}

void set_trigger(
    unsigned int seconds,
    const shared_ptr<TTransport>& transport,
    uint32_t write_len) {
  clear_triggers();
  add_trigger(seconds, transport, write_len);
}

/**************************************************************************
 * Test functions
 **************************************************************************/

/**
 * Test interleaved write and read calls.
 *
 * Generates a buffer totalSize bytes long, then writes it to the transport,
 * and verifies the written data can be read back correctly.
 *
 * Mode of operation:
 * - call wChunkGenerator to figure out how large of a chunk to write
 *   - call wSizeGenerator to get the size for individual write() calls,
 *     and do this repeatedly until the entire chunk is written.
 * - call rChunkGenerator to figure out how large of a chunk to read
 *   - call rSizeGenerator to get the size for individual read() calls,
 *     and do this repeatedly until the entire chunk is read.
 * - repeat until the full buffer is written and read back,
 *   then compare the data read back against the original buffer
 *
 *
 * - If any of the size generators return 0, this means to use the maximum
 *   possible size.
 *
 * - If maxOutstanding is non-zero, write chunk sizes will be chosen such that
 *   there are never more than maxOutstanding bytes waiting to be read back.
 */
template <class CoupledTransports>
void test_rw(
    uint32_t totalSize,
    GenericSizeGenerator wSizeGenerator,
    GenericSizeGenerator rSizeGenerator,
    GenericSizeGenerator wChunkGenerator = 0,
    GenericSizeGenerator rChunkGenerator = 0,
    uint32_t maxOutstanding = 0) {
  // adjust totalSize by the specified FLAGS_size_multiplier first
  totalSize = static_cast<uint32_t>(totalSize * FLAGS_size_multiplier);

  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  vector<uint8_t> wbuf(totalSize, 0);
  vector<uint8_t> rbuf(totalSize, 0);

  // store some data in wbuf
  for (uint32_t n = 0; n < totalSize; ++n) {
    wbuf[n] = (n & 0xff);
  }
  // clear rbuf
  memset(rbuf.data(), 0, totalSize);

  uint32_t total_written = 0;
  uint32_t total_read = 0;
  while (total_read < totalSize) {
    // Determine how large a chunk of data to write
    uint32_t wchunk_size = wChunkGenerator.nextSize();
    if (wchunk_size == 0 || wchunk_size > totalSize - total_written) {
      wchunk_size = totalSize - total_written;
    }

    // Make sure (total_written - total_read) + wchunk_size
    // is less than maxOutstanding
    if (maxOutstanding > 0 &&
        wchunk_size > maxOutstanding - (total_written - total_read)) {
      wchunk_size = maxOutstanding - (total_written - total_read);
    }

    // Write the chunk
    uint32_t chunk_written = 0;
    while (chunk_written < wchunk_size) {
      uint32_t write_size = wSizeGenerator.nextSize();
      if (write_size == 0 || write_size > wchunk_size - chunk_written) {
        write_size = wchunk_size - chunk_written;
      }

      transports.out->write(wbuf.data() + total_written, write_size);
      chunk_written += write_size;
      total_written += write_size;
    }

    // Flush the data, so it will be available in the read transport
    // Don't flush if wchunk_size is 0.  (This should only happen if
    // total_written == totalSize already, and we're only reading now.)
    if (wchunk_size > 0) {
      transports.out->flush();
    }

    // Determine how large a chunk of data to read back
    uint32_t rchunk_size = rChunkGenerator.nextSize();
    if (rchunk_size == 0 || rchunk_size > total_written - total_read) {
      rchunk_size = total_written - total_read;
    }

    // Read the chunk
    uint32_t chunk_read = 0;
    while (chunk_read < rchunk_size) {
      uint32_t read_size = rSizeGenerator.nextSize();
      if (read_size == 0 || read_size > rchunk_size - chunk_read) {
        read_size = rchunk_size - chunk_read;
      }

      int bytes_read = -1;
      try {
        bytes_read = transports.in->read(rbuf.data() + total_read, read_size);
      } catch (TTransportException& e) {
        ADD_FAILURE() << "read(pos=" << total_read << ", size=" << read_size
                      << ") threw exception \"" << e.what() << "\"; "
                      << "written so far: " << total_written << " / "
                      << totalSize << " bytes";
        throw;
      }

      ASSERT_GT(bytes_read, 0)
          << "read(pos=" << total_read << ", size=" << read_size << ") "
          << "returned " << bytes_read << "; written so far: " << total_written
          << " / " << totalSize << " bytes";
      chunk_read += bytes_read;
      total_read += bytes_read;
    }
  }

  // make sure the data read back is identical to the data written
  EXPECT_EQ(0, memcmp(rbuf.data(), wbuf.data(), totalSize));
}

template <class CoupledTransports>
void test_read_part_available() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  uint8_t read_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Attemping to read 10 bytes when only 9 are available should return 9
  // immediately.
  transports.out->write(write_buf, 9);
  transports.out->flush();
  set_trigger(3, transports.out, 1);
  uint32_t bytes_read = transports.in->read(read_buf, 10);
  EXPECT_EQ(0, numTriggersFired);
  EXPECT_EQ(9, bytes_read);

  clear_triggers();
}

template <class CoupledTransports>
void test_read_part_available_in_chunks() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  uint8_t read_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Write 10 bytes (in a single frame, for transports that use framing)
  transports.out->write(write_buf, 10);
  transports.out->flush();

  // Read 1 byte, to force the transport to read the frame
  uint32_t bytes_read = transports.in->read(read_buf, 1);
  EXPECT_EQ(1, bytes_read);

  // Read more than what is remaining and verify the transport does not block
  set_trigger(3, transports.out, 1);
  bytes_read = transports.in->read(read_buf, 10);
  EXPECT_EQ(0, numTriggersFired);
  EXPECT_EQ(9, bytes_read);

  clear_triggers();
}

template <class CoupledTransports>
void test_read_partial_midframe() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  uint8_t read_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Attempt to read 10 bytes, when only 9 are available, but after we have
  // already read part of the data that is available.  This exercises a
  // different code path for several of the transports.
  //
  // For transports that add their own framing (e.g., TFramedTransport and
  // TFileTransport), the two flush calls break up the data in to a 10 byte
  // frame and a 3 byte frame.  The first read then puts us partway through the
  // first frame, and then we attempt to read past the end of that frame, and
  // through the next frame, too.
  //
  // For buffered transports that perform read-ahead (e.g.,
  // TBufferedTransport), the read-ahead will most likely see all 13 bytes
  // written on the first read.  The next read will then attempt to read past
  // the end of the read-ahead buffer.
  //
  // Flush 10 bytes, then 3 bytes.  This creates 2 separate frames for
  // transports that track framing internally.
  transports.out->write(write_buf, 10);
  transports.out->flush();
  transports.out->write(write_buf, 3);
  transports.out->flush();

  // Now read 4 bytes, so that we are partway through the written data.
  uint32_t bytes_read = transports.in->read(read_buf, 4);
  EXPECT_EQ(4, bytes_read);

  // Now attempt to read 10 bytes.  Only 9 more are available.
  //
  // We should be able to get all 9 bytes, but it might take multiple read
  // calls, since it is valid for read() to return fewer bytes than requested.
  // (Most transports do immediately return 9 bytes, but the framing transports
  // tend to only return to the end of the current frame, which is 6 bytes in
  // this case.)
  uint32_t total_read = 0;
  while (total_read < 9) {
    set_trigger(3, transports.out, 1);
    bytes_read = transports.in->read(read_buf, 10);
    ASSERT_EQ(0, numTriggersFired);
    ASSERT_GT(bytes_read, 0);
    total_read += bytes_read;
    ASSERT_LE(total_read, 9);
  }

  EXPECT_EQ(9, total_read);

  clear_triggers();
}

template <class CoupledTransports>
void test_borrow_part_available() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  uint8_t read_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Attemping to borrow 10 bytes when only 9 are available should return NULL
  // immediately.
  transports.out->write(write_buf, 9);
  transports.out->flush();
  set_trigger(3, transports.out, 1);
  uint32_t borrow_len = 10;
  const uint8_t* borrowed_buf = transports.in->borrow(read_buf, &borrow_len);
  EXPECT_EQ(0, numTriggersFired);
  EXPECT_EQ(nullptr, borrowed_buf);

  clear_triggers();
}

template <class CoupledTransports>
void test_read_none_available() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  uint8_t read_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Attempting to read when no data is available should either block until
  // some data is available, or fail immediately.  (e.g., TSocket blocks,
  // TMemoryBuffer just fails.)
  //
  // If the transport blocks, it should succeed once some data is available,
  // even if less than the amount requested becomes available.
  set_trigger(1, transports.out, 2);
  add_trigger(1, transports.out, 8);
  uint32_t bytes_read = transports.in->read(read_buf, 10);
  if (bytes_read == 0) {
    EXPECT_EQ(0, numTriggersFired);
    clear_triggers();
  } else {
    EXPECT_EQ(1, numTriggersFired);
    EXPECT_EQ(2, bytes_read);
  }

  clear_triggers();
}

template <class CoupledTransports>
void test_borrow_none_available() {
  CoupledTransports transports;
  ASSERT_NE(nullptr, transports.in);
  ASSERT_NE(nullptr, transports.out);

  uint8_t write_buf[16];
  memset(write_buf, 'a', sizeof(write_buf));

  // Attempting to borrow when no data is available should fail immediately
  set_trigger(1, transports.out, 10);
  uint32_t borrow_len = 10;
  const uint8_t* borrowed_buf = transports.in->borrow(nullptr, &borrow_len);
  EXPECT_EQ(nullptr, borrowed_buf);
  EXPECT_EQ(0, numTriggersFired);

  clear_triggers();
}

class TransportTest : public testing::Test {
 public:
  TransportTest() { CHECK_GT(FLAGS_size_multiplier, 0); }
};

} // namespace

/**************************************************************************
 * Test case generation
 *
 * Pretty ugly and annoying.
 **************************************************************************/

#define TEST_RW_4_A(                                                            \
    Template, CoupledTransports, totalSize, wSizeGen, rSizeGen)                 \
  TEST_F(                                                                       \
      TransportTest,                                                            \
      Template##_##CoupledTransports##_##totalSize##_##wSizeGen##_##rSizeGen) { \
    test_rw<Template<CoupledTransports>>(totalSize, wSizeGen, rSizeGen);        \
  }

#define TEST_RW_4(CoupledTransports, ...)                                 \
  /* Add the test as specified, to test the non-virtual function calls */ \
  TEST_RW_4_A(IdentityT, CoupledTransports, __VA_ARGS__)                  \
  /*                                                                      \
   * Also test using the transport as a TTransport*, to test              \
   * the read_virt()/write_virt() calls                                   \
   */                                                                     \
  TEST_RW_4_A(CoupledTTransports, CoupledTransports, __VA_ARGS__)         \
  /* Test wrapping the transport with TBufferedTransport */               \
  TEST_RW_4_A(CoupledBufferedTransportsT, CoupledTransports, __VA_ARGS__) \
  /* Test wrapping the transport with TFramedTransports */                \
  TEST_RW_4_A(CoupledFramedTransportsT, CoupledTransports, __VA_ARGS__)   \
  /* Test wrapping the transport with TZlibTransport */                   \
  TEST_RW_4_A(CoupledZlibTransportsT, CoupledTransports, __VA_ARGS__)

#define TEST_RW_6_A(                                                                                                \
    Template,                                                                                                       \
    CoupledTransports,                                                                                              \
    totalSize,                                                                                                      \
    wSizeGen,                                                                                                       \
    rSizeGen,                                                                                                       \
    wChunkSizeGen,                                                                                                  \
    rChunkSizeGen)                                                                                                  \
  TEST_F(                                                                                                           \
      TransportTest,                                                                                                \
      Template##_##CoupledTransports##_##totalSize##_##wSizeGen##_##rSizeGen##_##wChunkSizeGen##_##rChunkSizeGen) { \
    test_rw<Template<CoupledTransports>>(                                                                           \
        totalSize, wSizeGen, rSizeGen, wChunkSizeGen, rChunkSizeGen);                                               \
  }

#define TEST_RW_6(CoupledTransports, ...)                                 \
  /* Add the test as specified, to test the non-virtual function calls */ \
  TEST_RW_6_A(IdentityT, CoupledTransports, __VA_ARGS__)                  \
  /*                                                                      \
   * Also test using the transport as a TTransport*, to test              \
   * the read_virt()/write_virt() calls                                   \
   */                                                                     \
  TEST_RW_6_A(CoupledTTransports, CoupledTransports, __VA_ARGS__)         \
  /* Test wrapping the transport with TBufferedTransport */               \
  TEST_RW_6_A(CoupledBufferedTransportsT, CoupledTransports, __VA_ARGS__) \
  /* Test wrapping the transport with TFramedTransports */                \
  TEST_RW_6_A(CoupledFramedTransportsT, CoupledTransports, __VA_ARGS__)   \
  /* Test wrapping the transport with TZlibTransport */                   \
  TEST_RW_6_A(CoupledZlibTransportsT, CoupledTransports, __VA_ARGS__)

#define TEST_RW_7_A(                                                                                                                   \
    Template,                                                                                                                          \
    CoupledTransports,                                                                                                                 \
    totalSize,                                                                                                                         \
    wSizeGen,                                                                                                                          \
    rSizeGen,                                                                                                                          \
    wChunkSizeGen,                                                                                                                     \
    rChunkSizeGen,                                                                                                                     \
    maxOutstanding)                                                                                                                    \
  TEST_F(                                                                                                                              \
      TransportTest,                                                                                                                   \
      Template##_##CoupledTransports##_##totalSize##_##wSizeGen##_##rSizeGen##_##wChunkSizeGen##_##rChunkSizeGen##_##maxOutstanding) { \
    test_rw<Template<CoupledTransports>>(                                                                                              \
        totalSize,                                                                                                                     \
        wSizeGen,                                                                                                                      \
        rSizeGen,                                                                                                                      \
        wChunkSizeGen,                                                                                                                 \
        rChunkSizeGen,                                                                                                                 \
        maxOutstanding);                                                                                                               \
  }

#define TEST_RW_7(CoupledTransports, ...)                                 \
  /* Add the test as specified, to test the non-virtual function calls */ \
  TEST_RW_7_A(IdentityT, CoupledTransports, __VA_ARGS__)                  \
  /*                                                                      \
   * Also test using the transport as a TTransport*, to test              \
   * the read_virt()/write_virt() calls                                   \
   */                                                                     \
  TEST_RW_7_A(CoupledTTransports, CoupledTransports, __VA_ARGS__)         \
  /* Test wrapping the transport with TBufferedTransport */               \
  TEST_RW_7_A(CoupledBufferedTransportsT, CoupledTransports, __VA_ARGS__) \
  /* Test wrapping the transport with TFramedTransports */                \
  TEST_RW_7_A(CoupledFramedTransportsT, CoupledTransports, __VA_ARGS__)   \
  /* Test wrapping the transport with TZlibTransport */                   \
  TEST_RW_7_A(CoupledZlibTransportsT, CoupledTransports, __VA_ARGS__)

#define TEST_BLOCKING_BEHAVIOR_B(Template, CoupledTransports, func)           \
  TEST_F(TransportTest, Blocking_##Template##_##CoupledTransports##_##func) { \
    test_##func<Template<CoupledTransports>>();                               \
  }

#define TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, func)                     \
  TEST_BLOCKING_BEHAVIOR_B(IdentityT, CoupledTransports, func)                \
  TEST_BLOCKING_BEHAVIOR_B(CoupledTTransports, CoupledTransports, func)       \
  TEST_BLOCKING_BEHAVIOR_B(                                                   \
      CoupledBufferedTransportsT, CoupledTransports, func)                    \
  TEST_BLOCKING_BEHAVIOR_B(CoupledFramedTransportsT, CoupledTransports, func) \
  TEST_BLOCKING_BEHAVIOR_B(CoupledZlibTransportsT, CoupledTransports, func)

#define TEST_BLOCKING_BEHAVIOR(CoupledTransports)                            \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, read_part_available)           \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, read_part_available_in_chunks) \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, read_partial_midframe)         \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, read_none_available)           \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, borrow_part_available)         \
  TEST_BLOCKING_BEHAVIOR_A(CoupledTransports, borrow_none_available)

static GenericSizeGenerator rand4k(1, 4096);
static constexpr size_t kConst16K = 1024 * 16;
static constexpr size_t kConst256K = 1024 * 256;
static constexpr size_t kConst1024K = 1024 * 1024;
static constexpr uint32_t kFdMaxOutstanding = 4096;
static constexpr uint32_t kSocketMaxOutstanding = 4096;

// TMemoryBuffer tests
TEST_RW_4(CoupledMemoryBuffers, kConst1024K, 0, 0)
TEST_RW_4(CoupledMemoryBuffers, kConst256K, rand4k, rand4k)
TEST_RW_4(CoupledMemoryBuffers, kConst256K, 167, 163)
TEST_RW_4(CoupledMemoryBuffers, kConst16K, 1, 1)

TEST_RW_6(CoupledMemoryBuffers, kConst256K, 0, 0, rand4k, rand4k)
TEST_RW_6(CoupledMemoryBuffers, kConst256K, rand4k, rand4k, rand4k, rand4k)
TEST_RW_6(CoupledMemoryBuffers, kConst256K, 167, 163, rand4k, rand4k)
TEST_RW_6(CoupledMemoryBuffers, kConst16K, 1, 1, rand4k, rand4k)

TEST_BLOCKING_BEHAVIOR(CoupledMemoryBuffers)

// TFDTransport tests
// Since CoupledFDTransports tests with a pipe, writes will block
// if there is too much outstanding unread data in the pipe.
TEST_RW_7(CoupledFDTransports, kConst1024K, 0, 0, 0, 0, kFdMaxOutstanding)
TEST_RW_7(
    CoupledFDTransports, kConst256K, rand4k, rand4k, 0, 0, kFdMaxOutstanding)
TEST_RW_7(CoupledFDTransports, kConst256K, 167, 163, 0, 0, kFdMaxOutstanding)
TEST_RW_7(CoupledFDTransports, kConst16K, 1, 1, 0, 0, kFdMaxOutstanding)

TEST_RW_7(
    CoupledFDTransports, kConst256K, 0, 0, rand4k, rand4k, kFdMaxOutstanding)
TEST_RW_7(
    CoupledFDTransports,
    kConst256K,
    rand4k,
    rand4k,
    rand4k,
    rand4k,
    kFdMaxOutstanding)
TEST_RW_7(
    CoupledFDTransports,
    kConst256K,
    167,
    163,
    rand4k,
    rand4k,
    kFdMaxOutstanding)
TEST_RW_7(
    CoupledFDTransports, kConst16K, 1, 1, rand4k, rand4k, kFdMaxOutstanding)

TEST_BLOCKING_BEHAVIOR(CoupledFDTransports)

// TSocket tests
TEST_RW_7(
    CoupledSocketTransports, kConst1024K, 0, 0, 0, 0, kSocketMaxOutstanding)
TEST_RW_7(
    CoupledSocketTransports,
    kConst256K,
    rand4k,
    rand4k,
    0,
    0,
    kSocketMaxOutstanding)
TEST_RW_7(
    CoupledSocketTransports, kConst256K, 167, 163, 0, 0, kSocketMaxOutstanding)
// Doh.  Apparently writing to a socket has some additional overhead for
// each send() call.  If we have more than ~100 outstanding 1-byte write
// requests, additional send() calls start blocking.
TEST_RW_7(CoupledSocketTransports, kConst16K, 1, 1, 0, 0, 100)
TEST_RW_7(
    CoupledSocketTransports,
    kConst256K,
    0,
    0,
    rand4k,
    rand4k,
    kSocketMaxOutstanding)
TEST_RW_7(
    CoupledSocketTransports,
    kConst256K,
    rand4k,
    rand4k,
    rand4k,
    rand4k,
    kSocketMaxOutstanding)
TEST_RW_7(
    CoupledSocketTransports,
    kConst256K,
    167,
    163,
    rand4k,
    rand4k,
    kSocketMaxOutstanding)
TEST_RW_7(CoupledSocketTransports, kConst16K, 1, 1, rand4k, rand4k, 100)

TEST_BLOCKING_BEHAVIOR(CoupledSocketTransports)

// Add some tests that access TBufferedTransport and TFramedTransport
// via TTransport pointers and TBufferBase pointers.
TEST_RW_6_A(
    CoupledTTransports,
    CoupledBufferedTransports,
    kConst1024K,
    rand4k,
    rand4k,
    rand4k,
    rand4k)
TEST_RW_6_A(
    CoupledBufferBases,
    CoupledBufferedTransports,
    kConst1024K,
    rand4k,
    rand4k,
    rand4k,
    rand4k)
TEST_RW_6_A(
    CoupledTTransports,
    CoupledFramedTransports,
    kConst1024K,
    rand4k,
    rand4k,
    rand4k,
    rand4k)
TEST_RW_6_A(
    CoupledBufferBases,
    CoupledFramedTransports,
    kConst1024K,
    rand4k,
    rand4k,
    rand4k,
    rand4k)

// Test using TZlibTransport via a TTransport pointer
TEST_RW_6_A(
    CoupledTTransports,
    CoupledZlibTransports,
    kConst1024K,
    rand4k,
    rand4k,
    rand4k,
    rand4k)
