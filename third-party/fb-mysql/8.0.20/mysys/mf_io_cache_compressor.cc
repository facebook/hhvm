/*
   Copyright (c) 2016, Facebook, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <new>
#include <stdexcept>
#include <vector>

#include <zstd.h>

#include "my_sys.h"
#include "sql/malloc_allocator.h"

static constexpr int compression_level = 3;  // ZSTD_CLEVEL_DEFAULT;

using byte_buffer = std::vector<uchar, Malloc_allocator<uchar>>;

class compressor {
 public:
  compressor(IO_CACHE *info, File file, size_t cachesize, cache_type type,
             my_off_t seek_offset, bool use_async_io, myf cache_myflags)
      : zstd_in_buf(ZSTD_CStreamInSize(),
                    Malloc_allocator<uchar>(PSI_NOT_INSTRUMENTED)),
        zstd_out_buf(ZSTD_CStreamOutSize(),
                     Malloc_allocator<uchar>(PSI_NOT_INSTRUMENTED)),
        cstream{ZSTD_createCStream()} {
    if (!cstream) throw std::bad_alloc();

    size_t zrc = ZSTD_initCStream(cstream.get(), compression_level);
    if (ZSTD_isError(zrc)) throw std::bad_alloc();

    if (init_io_cache(&cache, file, cachesize, type, seek_offset, use_async_io,
                      cache_myflags))
      throw std::runtime_error("error initializing file cache");
    info->write_pos = zstd_in_buf.data();
    info->write_end = zstd_in_buf.data() + zstd_in_buf.size();
  }

  compressor(const compressor &) = delete;
  compressor(compressor &&) = delete;
  compressor &operator=(const compressor &) = delete;
  compressor &operator=(compressor &&) = delete;

  int write(IO_CACHE *info, const uchar *buf, size_t length) noexcept {
    size_t buflen = info->write_pos - zstd_in_buf.data();

    if (compress_write_buffer(info, zstd_in_buf.data(), buflen)) return 1;

    info->write_pos = zstd_in_buf.data();

    if (length > zstd_in_buf.size()) {
      if (compress_write_buffer(info, buf, length)) return 1;
    } else {
      memcpy(info->write_pos, buf, length);
      info->write_pos += length;
    }
    return 0;
  }

  int end(IO_CACHE *info) noexcept {
    size_t buflen = info->write_pos - zstd_in_buf.data();
    int rc1 = compress_write_buffer(info, zstd_in_buf.data(), buflen);
    int rc2 = flush(info);
    int rc3 = end_io_cache(&cache);

    return rc1 || rc2 || rc3;
  }

  static int io_cache_compressor_write(IO_CACHE *info, const uchar *buf,
                                       size_t length) {
    if (info->compressor->write(info, buf, length)) return 1;
    return 0;
  }

 private:
  int compress_write_buffer(IO_CACHE *info, const uchar *buf,
                            size_t buflen) noexcept {
    ZSTD_inBuffer input = {buf, buflen, 0};
    while (input.pos < input.size) {
      ZSTD_outBuffer output = {zstd_out_buf.data(), zstd_out_buf.size(), 0};
      size_t zrc = ZSTD_compressStream(cstream.get(), &output, &input);
      if (ZSTD_isError(zrc)) {
        info->error = -1;
        return 1;
      }
      if (write_compressed_data(info, zstd_out_buf.data(), output.pos))
        return 1;
    }
    return 0;
  }

  int write_compressed_data(IO_CACHE *info, const uchar *buf,
                            size_t size) noexcept {
    if (size > 0 && my_b_write(&cache, buf, size)) {
      info->error = cache.error;
      return 1;
    }
    return 0;
  }

  int flush(IO_CACHE *info) noexcept {
    ZSTD_outBuffer output = {zstd_out_buf.data(), zstd_out_buf.size(), 0};
    size_t zrc = ZSTD_endStream(cstream.get(), &output);
    if (ZSTD_isError(zrc)) {
      info->error = -1;
      return 1;
    }
    if (write_compressed_data(info, zstd_out_buf.data(), output.pos)) return 1;
    return 0;
  }

  byte_buffer zstd_in_buf;
  byte_buffer zstd_out_buf;
  struct ZSTD_CStream_deleter {
    void operator()(ZSTD_CStream *ptr) const noexcept { ZSTD_freeCStream(ptr); }
  };
  using ZSTD_CStream_ptr = std::unique_ptr<ZSTD_CStream, ZSTD_CStream_deleter>;
  ZSTD_CStream_ptr cstream;
  IO_CACHE cache;
};

class decompressor {
 public:
  decompressor(IO_CACHE *info, File file, size_t cachesize, cache_type type,
               my_off_t seek_offset, bool use_async_io, myf cache_myflags)
      : zstd_out_buf(ZSTD_DStreamOutSize(),
                     Malloc_allocator<uchar>(PSI_NOT_INSTRUMENTED)),
        dstream{ZSTD_createDStream()} {
    if (!dstream) throw std::bad_alloc();

    size_t zrc = ZSTD_initDStream(dstream.get());
    if (ZSTD_isError(zrc)) throw std::bad_alloc();

    if (init_io_cache(&cache, file, cachesize, type, seek_offset, use_async_io,
                      cache_myflags)) {
      throw std::runtime_error("error initializing file cache");
    }
    input.pos = 0;
    input.size = 0;
    input.src = nullptr;
    internal_buffer_fully_flushed = true;
    info->request_pos = info->read_pos = info->read_end = zstd_out_buf.data();
  }
  decompressor(const decompressor &) = delete;
  decompressor(decompressor &&) = delete;
  decompressor &operator=(const decompressor &) = delete;
  decompressor &operator=(decompressor &&) = delete;

  void set_read_function(read_function_type read_function) {
    cache.read_function = read_function;
  }
  void set_preread_callback(IO_CACHE_CALLBACK preread) {
    cache.pre_read = preread;
  }
  void set_preclose_callback(IO_CACHE_CALLBACK preclose) {
    cache.pre_close = preclose;
  }
  void set_arg(void *arg) { cache.arg = arg; }

  int read(IO_CACHE *info, uchar *Buffer,
           size_t Count MY_ATTRIBUTE((unused))) noexcept {
    // _my_b_get() passes Count = 1
    DBUG_ASSERT(Count == 1);
    // output buffer must be empty, all data from there is consumed
    DBUG_ASSERT(info->read_pos == info->read_end);
    size_t read_data_length;
    while (true) {
      // if there is some data in decompressor's internal buffer, we flush it
      // if there is some data in input buffer, we decompress it
      // otherwise we read data from input source and decompress it

      // if decompressor internal empty && input buffers are empty, read input
      if (internal_buffer_fully_flushed && input.pos == input.size &&
          cache_read())
        return 1;  // EOF
      // set output buffer for decompression
      ZSTD_outBuffer output = {zstd_out_buf.data(), zstd_out_buf.size(), 0};
      size_t rc = ZSTD_decompressStream(dstream.get(), &output, &input);
      if (ZSTD_isError(rc)) {
        info->error = -1;
        return 1;
      }
      // check decompressor's internal buffer is fully flushed
      internal_buffer_fully_flushed = output.pos < output.size;
      if (output.pos > 0) {
        read_data_length = output.pos;
        break;
      }
      // output.pos == 0, output buffer is empty
    }
    // set output buffer pointers
    info->request_pos = info->read_pos = zstd_out_buf.data();
    info->read_end = zstd_out_buf.data() + read_data_length;
    DBUG_ASSERT(info->read_pos < info->read_end);
    // return first character
    Buffer[0] = info->read_pos[0];
    info->read_pos++;
    return 0;
  }

  int end() noexcept { return end_io_cache(&cache); }

 private:
  int cache_read() noexcept {
    // read data into buffer
    int rc = _my_b_get(&cache);
    if (rc == my_b_EOF) {
      return 1;  // EOF
    }
    // put back one byte read by _my_b_get()
    cache.read_pos--;

    uchar *in_buf = cache.read_pos;
    size_t in_buf_size = cache.read_end - cache.read_pos;
    // set read buffer consumed
    cache.read_pos = cache.read_end;

    DBUG_ASSERT(in_buf_size > 0);

    // set input buffer for decompression
    input.pos = 0;
    input.size = in_buf_size;
    input.src = in_buf;
    return 0;
  }

  byte_buffer zstd_out_buf;  // buffer for decompressed data
  struct ZSTD_DStream_deleter {
    void operator()(ZSTD_DStream *ptr) const noexcept { ZSTD_freeDStream(ptr); }
  };
  using ZSTD_DStream_ptr = std::unique_ptr<ZSTD_DStream, ZSTD_DStream_deleter>;
  ZSTD_DStream_ptr dstream;  // decompressor
  ZSTD_inBuffer input;       // pointer to data source buffer
  IO_CACHE cache;            // data source
  bool
      internal_buffer_fully_flushed;  // if EOF in data source,
                                      // but decompressor's internal buffer
                                      // still keep some data to decompress
                                      // because output buffer is not big enough
};

static int io_cache_compressor_write(IO_CACHE *info, const uchar *buf,
                                     size_t length) {
  if (info->compressor->write(info, buf, length)) return 1;
  return 0;
}

int end_io_cache_compressor(IO_CACHE *info) {
  int r = info->compressor->end(info);
  delete info->compressor;
  info->compressor = nullptr;
  return r;
}

// io_cache_decompressor_read() called from _my_b_get().
// _m_b_get() reads one byte at time.
// If IOCACHE buffer is empty, _m_b_get() calls read_function.
// LOAD INFILE sets read_function = _my_b_net_read()
// LOAD INFILE LOCAL set read_function = _my_b_read()
// LOAD INFILE [ LOCAL ] COMPRESSED sets
// read_function = io_cache_decompressor_read()
// We assume count == 1 because the READ_INFO code
// only calls my_b_get via the GET macro, and _my_b_net_read
// and io_cache_decompressor_read is making use of that.
static int io_cache_decompressor_read(IO_CACHE *info, uchar *Buffer,
                                      size_t Count) {
  if (info->decompressor->read(info, Buffer, Count)) return 1;
  return 0;
}

int end_io_cache_decompressor(IO_CACHE *info) {
  int r = info->decompressor->end();
  delete info->decompressor;
  info->decompressor = nullptr;
  return r;
}

int init_io_cache_with_opt_compression(IO_CACHE *info, File file,
                                       size_t cachesize, cache_type type,
                                       my_off_t seek_offset, bool use_async_io,
                                       myf cache_myflags, bool compressed) {
  if (!compressed)
    return init_io_cache(info, file, cachesize, type, seek_offset, use_async_io,
                         cache_myflags);

  *info = IO_CACHE{};

  try {
    if (type == WRITE_CACHE && cachesize == 0L && seek_offset == 0L) {
      info->compressor =
          new compressor(info, file, cachesize, type, seek_offset, use_async_io,
                         cache_myflags);
      info->write_function = io_cache_compressor_write;
    } else if ((type == READ_NET || type == READ_FIFO || type == READ_CACHE) &&
               cachesize == 0L && seek_offset == 0L) {
      info->decompressor =
          new decompressor(info, file, cachesize, type, seek_offset,
                           use_async_io, cache_myflags);
      info->read_function = io_cache_decompressor_read;
    } else {
      return 1;
    }
  } catch (const std::bad_alloc &) {
    info->error = -1;
    return 1;
  } catch (const std::exception &) {
    return 1;
  }

  info->file = -1;
  info->type = type;
  info->end_of_file = ~static_cast<my_off_t>(0);
  return 0;
}

void io_cache_set_read_function(IO_CACHE *cache,
                                read_function_type read_function) {
  decompressor *d = cache->decompressor;
  if (d != nullptr)
    d->set_read_function(read_function);
  else
    cache->read_function = read_function;
}
void io_cache_set_preread_callback(IO_CACHE *cache, IO_CACHE_CALLBACK preread) {
  decompressor *d = cache->decompressor;
  if (d != nullptr)
    d->set_preread_callback(preread);
  else
    cache->pre_read = preread;
}
void io_cache_set_preclose_callback(IO_CACHE *cache,
                                    IO_CACHE_CALLBACK preclose) {
  decompressor *d = cache->decompressor;
  if (d != nullptr)
    d->set_preclose_callback(preclose);
  else
    cache->pre_close = preclose;
}
void io_cache_set_arg(IO_CACHE *cache, void *arg) {
  decompressor *d = cache->decompressor;
  if (d != nullptr)
    d->set_arg(arg);
  else
    cache->arg = arg;
}
