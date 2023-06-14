/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <folly/String.h>
#include <proxygen/lib/http/codec/compress/HPACKCodec.h>
#include <proxygen/lib/http/codec/compress/NoPathIndexingStrategy.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionScheme.h>
#include <sys/queue.h>

#ifdef HAVE_REAL_QMIN
#include "qmin_common.h" // @manual
#include "qmin_dec.h"    // @manual
#include "qmin_enc.h"    // @manual
#else
/* Stub implementation for when you don't have QMIN */
extern "C" {
enum qmin_index_type {
  QIT_YES,
  QIT_NO,
  QIT_NEVER,
};
enum {
  QSIDE_CLIENT,
  QSIDE_SERVER,
};
enum qmin_encode_status {
  QES_OK,
  QES_NOBUFS,
  QES_ERR,
};
struct qmin_ctl_out {
  void (*qco_write)(void *qco_ctx, const void *, size_t);
  void *qco_ctx;
};
struct qmin_enc;
static struct qmin_enc *qmin_enc_new(int /*side*/,
                                     unsigned /*max_capacity*/,
                                     const struct qmin_ctl_out * /*ctl_out*/,
                                     const char * /*idstr*/) {
  return NULL;
}
static ssize_t qmin_enc_cmds_in(struct qmin_enc * /*enc*/,
                                const void * /*buf*/,
                                size_t /*bufsz*/) {
  return -1;
}
static enum qmin_encode_status qmin_enc_encode(struct qmin_enc * /*enc*/,
                                               unsigned /*stream_id*/,
                                               const char * /*name*/,
                                               unsigned /*name_len*/,
                                               const char * /*value*/,
                                               unsigned /*value_len*/,
                                               enum qmin_index_type /*ix_type*/,
                                               unsigned char * /*dst*/,
                                               size_t /*dst_sz*/,
                                               size_t * /*n_written*/) {
  return QES_ERR;
}
static int qmin_enc_end_stream_headers(struct qmin_enc * /*enc*/) {
  return -1;
}
static char *qmin_enc_to_str(struct qmin_enc * /*enc*/, size_t * /*size*/) {
  return strdup("");
}
static void qmin_enc_destroy(struct qmin_enc * /*enc*/) {
}
struct qmin_dec;
static struct qmin_dec *qmin_dec_new(int /*side*/,
                                     unsigned /*max_capacity*/,
                                     const struct qmin_ctl_out * /*ctl_out*/,
                                     const char * /*idstr*/) {
  return NULL;
}
static ssize_t qmin_dec_cmds_in(struct qmin_dec * /*dec*/,
                                const void * /*buf*/,
                                size_t /*bufsz*/) {
  return -1;
}
static ssize_t qmin_dec_decode(struct qmin_dec * /*dec*/,
                               const void * /*void_src*/,
                               size_t /*src_sz*/,
                               char * /*dst*/,
                               size_t /*dst_sz*/,
                               unsigned * /*name_len*/,
                               unsigned * /*val_len*/) {
  return -1;
}
static int qmin_dec_stream_done(struct qmin_dec * /*dec*/,
                                unsigned /*stream_id*/) {
  return -1;
}
static void qmin_dec_destroy(struct qmin_dec * /*dec*/) {
}
}
#endif

static unsigned s_seq;

TAILQ_HEAD(stream_chunks_head, stream_chunk);

struct stream_chunk {
  TAILQ_ENTRY(stream_chunk) sc_next;
  size_t sc_off;
  size_t sc_sz;
  unsigned char sc_buf[0];
};

struct stream {
  struct stream_chunks_head sm_chunks;
  size_t sm_read_off;
};

namespace proxygen { namespace compress {
class QMINScheme : public CompressionScheme {
 public:
  static struct stream_chunk *stream_chunk_new(size_t off,
                                               const void *buf,
                                               size_t bufsz) {
    struct stream_chunk *chunk =
        (struct stream_chunk *)malloc(sizeof(*chunk) + bufsz);
    assert(chunk);
    chunk->sc_off = off;
    chunk->sc_sz = bufsz;
    memcpy(chunk->sc_buf, buf, bufsz);
    return chunk;
  }

  static void insert_chunk(struct stream *stream,
                           struct stream_chunk *new_chunk) {
    struct stream_chunk *chunk;
    TAILQ_FOREACH(chunk, &stream->sm_chunks, sc_next)
    if (chunk->sc_off > new_chunk->sc_off) {
      TAILQ_INSERT_BEFORE(chunk, new_chunk, sc_next);
      return;
    }
    TAILQ_INSERT_TAIL(&stream->sm_chunks, new_chunk, sc_next);
  }

  static struct stream_chunk *maybe_pop_chunk(struct stream *stream) {
    struct stream_chunk *chunk = TAILQ_FIRST(&stream->sm_chunks);
    if (chunk && chunk->sc_off == stream->sm_read_off) {
      TAILQ_REMOVE(&stream->sm_chunks, chunk, sc_next);
      stream->sm_read_off += chunk->sc_sz;
      return chunk;
    } else
      return NULL;
  }

  explicit QMINScheme(CompressionSimulator *sim, uint32_t /*tableSize*/)
      : CompressionScheme(sim) {
    // TODO: set table size?
    qms_ctl[0].out.qco_write = write_enc2dec;
    qms_ctl[0].write_off = 0;
    qms_ctl[0].sz = 0;
    qms_ctl[1].out.qco_write = write_dec2enc;
    qms_ctl[1].write_off = 0;
    qms_ctl[1].sz = 0;

    qms_idstr = (char *)malloc(8);
    sprintf(qms_idstr, "%u", s_seq++);

    qms_enc = qmin_enc_new(QSIDE_CLIENT, 4 * 1024, &qms_ctl[0].out, qms_idstr);
    qms_dec = qmin_dec_new(QSIDE_SERVER, 4 * 1024, &qms_ctl[1].out, qms_idstr);

    qms_streams = (struct stream *)calloc(2, sizeof(qms_streams[0]));
    TAILQ_INIT(&qms_streams[0].sm_chunks);
    TAILQ_INIT(&qms_streams[1].sm_chunks);

    qms_next_stream_id_to_encode = 1;
  }

  ~QMINScheme() {
    free(qms_streams);
    qmin_enc_destroy(qms_enc);
    qmin_dec_destroy(qms_dec);
    free(qms_idstr);
  }

  /* QMIN Ack carries QMM_STREAM_DONE and QMM_ACK_FLUSH messages from decoder
   * to the encoder.
   */
  struct QMINAck : public CompressionScheme::Ack {
    explicit QMINAck(size_t off, const void *buf, size_t bufsz) {
      qma_off = off;
      qma_sz = bufsz;
      memcpy(qma_buf, buf, bufsz);
    }

    size_t qma_off;
    size_t qma_sz;
    unsigned char qma_buf[0x1000];
  };

  std::unique_ptr<Ack> getAck(uint16_t /*seqn*/) override {
    if (qms_ctl[1].sz) {
      auto ack = std::make_unique<QMINAck>(
          qms_ctl[1].write_off, qms_ctl[1].buf, qms_ctl[1].sz);
      VLOG(4) << "sent ACK for instance " << qms_idstr
              << " off: " << qms_ctl[1].write_off << "; sz: " << qms_ctl[1].sz;
      qms_ctl[1].write_off += qms_ctl[1].sz;
      qms_ctl[1].sz = 0;
      return std::move(ack);
    } else {
      assert(0);
      return nullptr;
    }
  }

  void recvAck(std::unique_ptr<Ack> generic_ack) override {
    struct stream_chunk *chunk;

    CHECK(generic_ack);
    auto ack = dynamic_cast<QMINAck *>(generic_ack.get());
    CHECK_NOTNULL(ack);

    VLOG(4) << "received ACK for instance " << qms_idstr
            << " off: " << ack->qma_off << "; sz: " << ack->qma_sz;

    chunk = stream_chunk_new(ack->qma_off, ack->qma_buf, ack->qma_sz);
    insert_chunk(&qms_streams[0], chunk);

    while ((chunk = maybe_pop_chunk(&qms_streams[0]))) {
      ssize_t nread;
      nread = qmin_enc_cmds_in(qms_enc, chunk->sc_buf, chunk->sc_sz);
      if (nread < 0 || (size_t)nread != chunk->sc_sz) {
        VLOG(1) << "error: qmin_enc_cmds_in failed";
        assert(0);
      }
      free(chunk);
    }
  }

  std::pair<FrameFlags, std::unique_ptr<folly::IOBuf>> encode(
      bool /*newPacket*/,
      std::vector<compress::Header> allHeaders,
      SimStats &stats) override {
    const size_t max_ctl = 0x1000;
    const size_t max_comp = 0x1000;
    unsigned char outbuf[max_ctl + max_comp];
    unsigned char *const comp = outbuf + max_ctl;
    size_t nw, comp_sz;
    enum qmin_encode_status qes;
    FrameFlags flags;

    qms_ctl[0].out.qco_ctx = this;
    comp_sz = 0;

    for (const auto header : allHeaders) {
      std::string name{header.name->c_str()};
      folly::toLowerAscii(name);
      qes = qmin_enc_encode(qms_enc,
                            qms_next_stream_id_to_encode,
                            name.c_str(),
                            name.length(),
                            header.value->c_str(),
                            header.value->length(),
                            QIT_YES,
                            comp + comp_sz,
                            max_comp - comp_sz,
                            &nw);
      switch (qes) {
        case QES_OK:
          /* 2 is a magic number added to the uncompressed size by the other
           * encoder.  We follow suit to make the numbers match.
           */
          stats.uncompressed += name.length() + header.value->length() + 2;
          stats.compressed += nw;
          comp_sz += nw;
          break;
        case QES_NOBUFS:
          VLOG(1) << "compressed header does not fit into temporary "
                     "output buffer";
          return {flags, nullptr};
        case QES_ERR:
          VLOG(1) << "error: " << strerror(errno);
          assert(0);
          return {flags, nullptr};
      }
    }

    {
      size_t sz;
      char *state = qmin_enc_to_str(qms_enc, &sz);
      VLOG(4) << "encoder state: " << state;
      free(state);
    }

    if (0 != qmin_enc_end_stream_headers(qms_enc)) {
      VLOG(1) << "error: qmin_enc_end_stream_headers failed";
      assert(0);
    }

    /* Prepend control message and its size: */
    size_t ctl_msg_sz = qms_ctl[0].sz;
    qms_ctl[0].sz = 0;
    size_t ctl_msg_sz_with_off;
    if (ctl_msg_sz) {
      memcpy(outbuf + max_ctl - ctl_msg_sz, qms_ctl[0].buf, ctl_msg_sz);
      memcpy(outbuf + max_ctl - ctl_msg_sz - sizeof(qms_ctl[0].write_off),
             &qms_ctl[0].write_off,
             sizeof(qms_ctl[0].write_off));
      qms_ctl[0].write_off += ctl_msg_sz;
      ctl_msg_sz_with_off = ctl_msg_sz + sizeof(qms_ctl[0].write_off);
    } else
      ctl_msg_sz_with_off = 0;
    memcpy(outbuf + max_ctl - ctl_msg_sz_with_off - sizeof(ctl_msg_sz),
           &ctl_msg_sz,
           sizeof(ctl_msg_sz));

    stats.compressed += ctl_msg_sz;

    /* Prepend Stream ID: */
    memcpy(outbuf + max_ctl - ctl_msg_sz_with_off - sizeof(ctl_msg_sz) -
               sizeof(uint32_t),
           &qms_next_stream_id_to_encode,
           sizeof(qms_next_stream_id_to_encode));

    qms_next_stream_id_to_encode += 2;
    flags.allowOOO = true;
    return {
        flags,
        folly::IOBuf::copyBuffer(outbuf + max_ctl - ctl_msg_sz_with_off -
                                     sizeof(ctl_msg_sz) - sizeof(uint32_t),
                                 comp_sz + ctl_msg_sz_with_off +
                                     sizeof(ctl_msg_sz) + sizeof(uint32_t))};
  }

  void decode(FrameFlags,
              std::unique_ptr<folly::IOBuf> encodedReq,
              SimStats &,
              SimStreamingCallback &callback) override {
    folly::io::Cursor cursor(encodedReq.get());
    const unsigned char *buf;
    ssize_t nread;
    size_t ctl_sz, stream_off;
    char outbuf[0x1000];
    unsigned name_len, val_len;
    unsigned decoded_size = 0;
    uint32_t stream_id;

    qms_ctl[1].out.qco_ctx = this;

    /* Read Stream ID: */
    buf = cursor.data();
    memcpy(&stream_id, buf, sizeof(uint32_t));
    encodedReq->trimStart(sizeof(uint32_t));

    /* Read size of control messages */
    buf = cursor.data();
    memcpy(&ctl_sz, buf, sizeof(ctl_sz));
    encodedReq->trimStart(sizeof(ctl_sz));

    /* Feed control messages to the decoder: */
    if (ctl_sz) {
      struct stream_chunk *chunk;

      /* Read stream offset: */
      buf = cursor.data();
      memcpy(&stream_off, buf, sizeof(stream_off));
      encodedReq->trimStart(sizeof(stream_off));

      buf = cursor.data();
      chunk = stream_chunk_new(stream_off, buf, ctl_sz);
      encodedReq->trimStart(ctl_sz);

      insert_chunk(&qms_streams[1], chunk);

      while ((chunk = maybe_pop_chunk(&qms_streams[1]))) {
        nread = qmin_dec_cmds_in(qms_dec, chunk->sc_buf, chunk->sc_sz);
        if (nread < 0 || (size_t)nread != chunk->sc_sz) {
          VLOG(1) << "error: qmin_dec_cmds_in failed";
          assert(0);
        }
        free(chunk);
      }
    }

    buf = cursor.data();
    const unsigned char *const end = buf + cursor.length();

    while (buf < end) {
      nread = qmin_dec_decode(
          qms_dec, buf, end - buf, outbuf, sizeof(outbuf), &name_len, &val_len);
      if (nread < 0) {
        VLOG(1) << "error: decoder failed!";
        assert(0);
        return;
      }
      assert(nread);
      buf += nread;
      decoded_size += name_len + val_len;
      std::string name{outbuf, name_len};
      std::string value{outbuf + name_len, val_len};
      callback.onHeader(HPACKHeaderName(folly::StringPiece(name)), value);
    }

    if (0 != qmin_dec_stream_done(qms_dec, stream_id)) {
      assert(0);
      VLOG(1) << "error: qmin_dec_stream_done failed";
    }

    proxygen::HTTPHeaderSize sz;
    sz.compressed = encodedReq->computeChainDataLength();
    sz.uncompressed = decoded_size;
    callback.onHeadersComplete(sz, true);
  }

  uint32_t getHolBlockCount() const override {
    return 0;
  }

  void runLoopCallback() noexcept override {
    CompressionScheme::runLoopCallback();
  }

  void write_ctl_msg(const void *buf, size_t sz, unsigned idx) {
    size_t avail = sizeof(qms_ctl[idx].buf) - qms_ctl[idx].sz;
    assert(avail >= sz);
    if (avail < sz) {
      VLOG(1) << "Truncating control message from " << sz << " to " << avail
              << "bytes";
      sz = avail;
    }
    memcpy(qms_ctl[idx].buf + qms_ctl[idx].sz, buf, sz);
    qms_ctl[idx].sz += sz;
    VLOG(4) << "Wrote " << sz << " bytes to control channel";
  }

  static void write_enc2dec(void *ctx, const void *buf, size_t sz) {
    QMINScheme *const qms = (QMINScheme *)ctx;
    qms->write_ctl_msg(buf, sz, 0);
  }

  static void write_dec2enc(void *ctx, const void *buf, size_t sz) {
    QMINScheme *const qms = (QMINScheme *)ctx;
    qms->write_ctl_msg(buf, sz, 1);
  }

  char *qms_idstr;

  struct qmin_enc *qms_enc;
  struct qmin_dec *qms_dec;

  /* Each call to `encode' is interpreted as a header block for a new
   * stream.
   */
  unsigned qms_next_stream_id_to_encode;

  /* 0: decoder-to-encoder; 1: encoder-to-decoder */
  struct stream *qms_streams;

  struct {
    struct qmin_ctl_out out;
    size_t write_off;
    size_t sz;
    unsigned char buf[0x1000];
  } qms_ctl[2]; /* 0: enc-to-dec; 1: dec-to-enc */
};
}} // namespace proxygen::compress
