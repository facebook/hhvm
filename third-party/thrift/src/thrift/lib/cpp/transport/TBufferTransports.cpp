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

#include <algorithm>
#include <cassert>

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>

using std::string;

namespace apache::thrift::transport {

void TBufferedTransport::putBack(uint8_t* buf, uint32_t len) {
  assert(len < rBufSize_);

  // Check that len fits.
  // We push rBase_ back below to rBuf_.get()
  if (rBound_ + len > rBufSize_ + rBase_) {
    throw TTransportException(
        TTransportException::BAD_ARGS,
        "TBufferedTransport called with oversize buf");
  }

  // Reset the buffer to initial position, moving any unread data.
  if (rBuf_.get() + len > rBase_) {
    if (rBase_ != rBound_) {
      // advance further to get room for the putback bytes
      memmove(rBuf_.get() + len, rBase_, rBound_ - rBase_);
    }
    rBound_ += (rBuf_.get() - rBase_);
    rBase_ = rBuf_.get();
  } else {
    rBase_ -= len;
    rBound_ -= len;
  }
  memcpy(rBase_, buf, len);
  rBound_ += len;
}

uint32_t TBufferedTransport::readSlow(uint8_t* buf, uint32_t len) {
  uint32_t have = rBound_ - rBase_;

  // We should only take the slow path if we can't satisfy the read
  // with the data already in the buffer.
  assert(have < len);

  // If we have some data in the buffer, copy it out and return it.
  // We have to return it without attempting to read more, since we aren't
  // guaranteed that the underlying transport actually has more data, so
  // attempting to read from it could block.
  if (have > 0) {
    memcpy(buf, rBase_, have);
    setReadBuffer(rBuf_.get(), 0);
    return have;
  }

  // No data is available in our buffer.
  // Get more from underlying transport up to buffer size.
  // Note that this makes a lot of sense if len < rBufSize_
  // and almost no sense otherwise.  TODO(dreiss): Fix that
  // case (possibly including some readv hotness).
  setReadBuffer(rBuf_.get(), transport_->read(rBuf_.get(), rBufSize_));

  // Hand over whatever we have.
  uint32_t give = std::min(len, static_cast<uint32_t>(rBound_ - rBase_));
  memcpy(buf, rBase_, give);
  rBase_ += give;

  return give;
}

void TBufferedTransport::writeSlow(const uint8_t* buf, uint32_t len) {
  uint32_t have_bytes = wBase_ - wBuf_.get();
  uint32_t space = wBound_ - wBase_;
  // We should only take the slow path if we can't accommodate the write
  // with the free space already in the buffer.
  assert(wBound_ - wBase_ < static_cast<ptrdiff_t>(len));

  // Now here's the tricky question: should we copy data from buf into our
  // internal buffer and write it from there, or should we just write out
  // the current internal buffer in one syscall and write out buf in another.
  // If our currently buffered data plus buf is at least double our buffer
  // size, we will have to do two syscalls no matter what (except in the
  // degenerate case when our buffer is empty), so there is no use copying.
  // Otherwise, there is sort of a sliding scale.  If we have N-1 bytes
  // buffered and need to write 2, it would be crazy to do two syscalls.
  // On the other hand, if we have 2 bytes buffered and are writing 2N-3,
  // we can save a syscall in the short term by loading up our buffer, writing
  // it out, and copying the rest of the bytes into our buffer.  Of course,
  // if we get another 2-byte write, we haven't saved any syscalls at all,
  // and have just copied nearly 2N bytes for nothing.  Finding a perfect
  // policy would require predicting the size of future writes, so we're just
  // going to always eschew syscalls if we have less than 2N bytes to write.

  // The case where we have to do two syscalls.
  // This case also covers the case where the buffer is empty,
  // but it is clearer (I think) to think of it as two separate cases.
  if ((have_bytes + len >= 2 * wBufSize_) || (have_bytes == 0)) {
    // TODO(dreiss): writev
    if (have_bytes > 0) {
      transport_->write(wBuf_.get(), have_bytes);
    }
    transport_->write(buf, len);
    wBase_ = wBuf_.get();
    return;
  }

  // Fill up our internal buffer for a write.
  memcpy(wBase_, buf, space);
  buf += space;
  len -= space;
  transport_->write(wBuf_.get(), wBufSize_);

  // Copy the rest into our buffer.
  assert(len < wBufSize_);
  memcpy(wBuf_.get(), buf, len);
  wBase_ = wBuf_.get() + len;
  return;
}

const uint8_t* TBufferedTransport::borrowSlow(
    uint8_t* /*buf*/, uint32_t* /*len*/) {
  // Simply return nullptr.  We don't know if there is actually data available
  // on the underlying transport, so calling read() might block.
  return nullptr;
}

void TBufferedTransport::flush() {
  // Write out any data waiting in the write buffer.
  uint32_t have_bytes = wBase_ - wBuf_.get();
  if (have_bytes > 0) {
    // Note that we reset wBase_ prior to the underlying write
    // to ensure we're in a sane state (i.e. internal buffer cleaned)
    // if the underlying write throws up an exception
    wBase_ = wBuf_.get();
    transport_->write(wBuf_.get(), have_bytes);
  }

  // Flush the underlying transport.
  transport_->flush();

  // Shrink the buffer to the reset size if we went above that size.
  shrinkWriteBuffer();
}

void TBufferedTransport::shrinkWriteBuffer() {
  if (wBufResetEveryN_ != 0 && wBufResetEveryN_ == ++wBufResetCount_) {
    wBufResetCount_ = 0;
    if (wBufSize_ > wBufResetSize_) {
      uint8_t* new_buf = new uint8_t[wBufResetSize_];
      wBuf_.reset(new_buf);
      wBufSize_ = wBufResetSize_;
      setWriteBuffer(wBuf_.get(), wBufSize_);
    }
  }
}

uint32_t TFramedTransport::readSlow(uint8_t* buf, uint32_t len) {
  uint32_t want = len;
  uint32_t have = rBound_ - rBase_;

  // We should only take the slow path if we can't satisfy the read
  // with the data already in the buffer.
  assert(have < want);

  // If we have some data in the buffer, copy it out and return it.
  // We have to return it without attempting to read more, since we aren't
  // guaranteed that the underlying transport actually has more data, so
  // attempting to read from it could block.
  if (have > 0) {
    memcpy(buf, rBase_, have);
    setReadBuffer(rBuf_.get(), 0);
    return have;
  }

  // Read another frame.
  if (!readFrame(len)) {
    // EOF.  No frame available.
    return 0;
  }

  // TODO(dreiss): Should we warn when reads cross frames?

  // Hand over whatever we have.
  uint32_t give = std::min(want, static_cast<uint32_t>(rBound_ - rBase_));
  memcpy(buf, rBase_, give);
  rBase_ += give;
  want -= give;

  return (len - want);
}

bool TFramedTransport::readFrame(uint32_t /*minFrameSize*/) {
  // TODO(dreiss): Think about using readv here, even though it would
  // result in (gasp) read-ahead.

  // Read the size of the next frame.
  // We can't use readAll(&sz, sizeof(sz)), since that always throws an
  // exception on EOF.  We want to throw an exception only if EOF occurs after
  // partial size data.
  uint32_t sz = 0;
  uint32_t size_bytes_read = 0;
  while (size_bytes_read < sizeof(sz)) {
    uint8_t* szp = reinterpret_cast<uint8_t*>(&sz) + size_bytes_read;
    uint32_t bytes_read = transport_->read(szp, sizeof(sz) - size_bytes_read);
    if (bytes_read == 0) {
      if (size_bytes_read == 0) {
        // EOF before any data was read.
        return false;
      } else {
        // EOF after a partial frame header.  Raise an exception.
        throw TTransportException(
            TTransportException::END_OF_FILE,
            "No more data to read after "
            "partial frame header.");
      }
    }
    size_bytes_read += bytes_read;
  }

  sz = folly::Endian::big(sz);

  if (sz > maxFrameSize_) {
    throw TTransportException("Frame size exceeded maximum");
  }

  // Read the frame payload, and reset markers.
  if (sz > rBufSize_) {
    rBuf_.reset(new uint8_t[sz]);
    rBufSize_ = sz;
  }
  transport_->readAll(rBuf_.get(), sz);
  setReadBuffer(rBuf_.get(), sz);
  return true;
}

void TFramedTransport::writeSlow(const uint8_t* buf, uint32_t len) {
  // Double buffer size until sufficient.
  uint32_t have = wBase_ - wBuf_.get();
  uint32_t new_size = wBufSize_;
  if (len + have < have /* overflow */ || len + have > 0x7fffffff) {
    throw TTransportException(
        TTransportException::BAD_ARGS,
        "Attempted to write over 2 GB to TFramedTransport.");
  }
  while (new_size < len + have) {
    new_size = new_size > 0 ? new_size * 2 : 1;
  }

  // TODO(dreiss): Consider modifying this class to use malloc/free
  // so we can use realloc here.

  // Allocate new buffer.
  uint8_t* new_buf = new uint8_t[new_size];

  // Copy the old buffer to the new one.
  memcpy(new_buf, wBuf_.get(), have);

  // Now point buf to the new one.
  wBuf_.reset(new_buf);
  wBufSize_ = new_size;
  wBase_ = wBuf_.get() + have;
  wBound_ = wBuf_.get() + wBufSize_;

  // Copy the data into the new buffer.
  memcpy(wBase_, buf, len);
  wBase_ += len;
}

void TFramedTransport::flush() {
  int32_t sz_hbo, sz_nbo;
  assert(wBufSize_ > sizeof(sz_nbo));

  // Slip the frame size into the start of the buffer.
  sz_hbo = wBase_ - (wBuf_.get() + sizeof(sz_nbo));
  sz_nbo = folly::Endian::big(sz_hbo);
  memcpy(wBuf_.get(), (uint8_t*)&sz_nbo, sizeof(sz_nbo));

  if (sz_hbo > 0) {
    // Note that we reset wBase_ (with a pad for the frame size)
    // prior to the underlying write to ensure we're in a sane state
    // (i.e. internal buffer cleaned) if the underlying write throws
    // up an exception
    wBase_ = wBuf_.get() + sizeof(sz_nbo);

    // Write size and frame body.
    transport_->write(wBuf_.get(), sizeof(sz_nbo) + sz_hbo);
  }

  // Flush the underlying transport.
  transport_->flush();

  // Shrink the buffer to the reset size if we went above that size.
  shrinkWriteBuffer();
}

void TFramedTransport::shrinkWriteBuffer() {
  if (wBufResetEveryN_ != 0 && wBufResetEveryN_ == ++wBufResetCount_) {
    wBufResetCount_ = 0;
    if (wBufSize_ > wBufResetSize_) {
      uint8_t* new_buf = new uint8_t[wBufResetSize_];
      uint32_t padding = wBase_ - wBuf_.get();
      wBuf_.reset(new_buf);
      wBufSize_ = wBufResetSize_;
      wBase_ = wBuf_.get() + padding;
      wBound_ = wBuf_.get() + wBufSize_;
    }
  }
}

uint32_t TFramedTransport::writeEnd() {
  return wBase_ - wBuf_.get();
}

const uint8_t* TFramedTransport::borrowSlow(
    uint8_t* /*buf*/, uint32_t* /*len*/) {
  // Don't try to be clever with shifting buffers.
  // If the fast path failed let the protocol use its slow path.
  // Besides, who is going to try to borrow across messages?
  return nullptr;
}

uint32_t TFramedTransport::readEnd() {
  // include framing bytes
  return rBound_ - rBuf_.get() + sizeof(uint32_t);
}

void TMemoryBuffer::computeRead(
    uint32_t len, uint8_t** out_start, uint32_t* out_give) {
  // Correct rBound_ so we can use the fast path in the future.
  rBound_ = wBase_;

  // Decide how much to give.
  uint32_t give = std::min(len, available_read());

  *out_start = rBase_;
  *out_give = give;

  // Preincrement rBase_ so the caller doesn't have to.
  rBase_ += give;
}

uint32_t TMemoryBuffer::readSlow(uint8_t* buf, uint32_t len) {
  uint8_t* start;
  uint32_t give;
  computeRead(len, &start, &give);

  // Copy into the provided buffer.
  if (start) {
    memcpy(buf, start, give);
  }

  return give;
}

uint32_t TMemoryBuffer::readAppendToString(std::string& str, uint32_t len) {
  // Don't get some stupid assertion failure.
  if (buffer_ == nullptr) {
    return 0;
  }

  uint8_t* start;
  uint32_t give;
  computeRead(len, &start, &give);

  // Append to the provided string.
  str.append((char*)start, give);

  return give;
}

void TMemoryBuffer::ensureCanWrite(uint32_t len) {
  // Check available space
  uint32_t avail = available_write();
  if (len <= avail) {
    return;
  }

  if (!owner_) {
    throw TTransportException("Insufficient space in external MemoryBuffer");
  }

  bool copy = false;
  if (observerCount_ > 0) {
    transferOwnership();
    // going to make a copy and shift out consumed data
    copy = true;
  }
  avail = bufferSize_ - available_read();

  // Grow the buffer as necessary.
  uint32_t new_size = bufferSize_;
  while (len > avail) {
    new_size = new_size > 0 ? new_size * 2 : 1;
    if (new_size <= bufferSize_) {
      // overflow
      throw TTransportException("Buffer size exceeded maximum (2GB)");
    }
    avail = available_write() + (new_size - bufferSize_);
  }

  // Allocate into a new pointer so we don't bork ours if it fails.
  uint8_t* new_buffer = nullptr;
  if (copy) {
    /* This is not a memory leak, because an observed buffer still owns the
     * previous buffer pointer.
     *
     * NOTE: If you are seeing a memory issue here, it's probable the
     * buffers are just growing very large and not being freed often.
     * Consider setting setResizeBufferEveryN,
     * setIdleWriteBufferLimit, and setIdleReadBufferLimit.
     * This will likely solve the issue.
     */
    new_buffer = static_cast<uint8_t*>(std::malloc(new_size));
    if (new_buffer == nullptr) {
      throw std::bad_alloc();
    }
    // only copy the unconsumed data
    memcpy(new_buffer, rBase_, wBase_ - rBase_);
  } else {
    if (rBase_ != buffer_) {
      // shift data
      memmove(buffer_, rBase_, wBase_ - rBase_);
    }
    if (new_size > bufferSize_) {
      new_buffer = static_cast<uint8_t*>(std::realloc(buffer_, new_size));
      if (new_buffer == nullptr) {
        throw std::bad_alloc();
      }
    } else {
      // plenty of space
      new_buffer = buffer_;
    }
  }
  ptrdiff_t rBoundOffset = rBound_ - rBase_;
  ptrdiff_t wBaseOffset = wBase_ - rBase_;
  bufferSize_ = new_size;

  buffer_ = new_buffer;
  rBase_ = new_buffer;
  rBound_ = new_buffer + rBoundOffset;
  wBase_ = new_buffer + wBaseOffset;
  wBound_ = buffer_ + bufferSize_;
}

void TMemoryBuffer::writeSlow(const uint8_t* buf, uint32_t len) {
  ensureCanWrite(len);

  // Copy into the buffer and increment wBase_.
  memcpy(wBase_, buf, len);
  wBase_ += len;
}

void TMemoryBuffer::wroteBytes(uint32_t len) {
  uint32_t avail = available_write();
  if (len > avail) {
    throw TTransportException("Client wrote more bytes than size of buffer.");
  }
  wBase_ += len;
}

const uint8_t* TMemoryBuffer::borrowSlow(uint8_t* /*buf*/, uint32_t* len) {
  rBound_ = wBase_;
  if (available_read() >= *len) {
    *len = available_read();
    return rBase_;
  }
  return nullptr;
}

} // namespace apache::thrift::transport
