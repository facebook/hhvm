/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <deque>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/io/Cursor.h>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/SettingsId.h>
#include <proxygen/lib/utils/Export.h>
#include <string.h>

#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace proxygen { namespace http2 {

//////// Constants ////////

extern const uint8_t kMinExperimentalFrameType;
using Padding = folly::Optional<uint8_t>;
extern const Padding kNoPadding;

//////// Types ////////

enum class FrameType : uint8_t {
  DATA = 0,
  HEADERS = 1,
  PRIORITY = 2,
  RST_STREAM = 3,
  SETTINGS = 4,
  PUSH_PROMISE = 5,
  PING = 6,
  GOAWAY = 7,
  WINDOW_UPDATE = 8,
  CONTINUATION = 9,
  ALTSVC = 10, // not in current draft so frame type has not been assigned

  // experimental use
  EX_HEADERS = 0xfb,

  // For secondary certificate authentication in HTTP/2 as specified in the
  // draft-ietf-httpbis-http2-secondary-certs-02.
  CERTIFICATE_REQUEST = 0xf0,
  CERTIFICATE = 0xf1,
  CERTIFICATE_NEEDED = 0xf2,
  USE_CERTIFICATE = 0xf3,
};

enum Flags {
  ACK = 0x1,
  END_STREAM = 0x1,
  END_HEADERS = 0x4,
  PADDED = 0x8,
  PRIORITY = 0x20,
  // experimental flag for EX stream only
  UNIDIRECTIONAL = 0x40,

  // for secondary certificate authentication frames
  UNSOLICITED = 0x1,
  TO_BE_CONTINUED = 0x1,
};

struct FrameHeader {
  uint32_t length; // only 24 valid bits
  uint32_t stream;
  FrameType type;
  uint8_t flags;
  uint16_t unused;
};

static_assert(sizeof(FrameHeader) == 12, "The maths are not working");

struct PriorityUpdate {
  // StreamID is 64bit integer to accommodate both HTTP2 and HTTP3 streams so
  // just validate the id for HTTP2 before encoding it on the wire
  HTTPCodec::StreamID streamDependency;
  bool exclusive;
  uint8_t weight;
};

//////// Bonus Constant ////////

FB_EXPORT extern const PriorityUpdate DefaultPriority;

//////// Functions ////////

bool isValidFrameType(FrameType t);

bool frameAffectsCompression(FrameType t);

/**
 * This function returns true if the padding bit is set in the header
 *
 * @param header The frame header.
 * @return true if the padding bit is set, false otherwise.
 */
bool frameHasPadding(const FrameHeader& header);

//// Parsing ////

/**
 * This function parses the common HTTP/2 frame header. This function
 * pulls kFrameHeaderSize bytes from the cursor, so the caller must check
 * that that amount is available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header struct to populate.
 * @return Nothing if success. The connection error code if failure.
 */
ErrorCode parseFrameHeader(folly::io::Cursor& cursor,
                           FrameHeader& header) noexcept;

/**
 * This function parses the section of the DATA frame after the common
 * frame header. It discards any padding and returns the body data in
 * outBuf. It pulls header.length bytes from the cursor, so it is the
 * caller's responsibility to ensure there is enough data available.
 *
 * @param cursor  The cursor to pull data from.
 * @param header  The frame header for the frame being parsed.
 * @param outBuf  The buf to fill with body data.
 * @param padding The number of padding bytes in this data frame
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseData(folly::io::Cursor& cursor,
                    const FrameHeader& header,
                    std::unique_ptr<folly::IOBuf>& outBuf,
                    uint16_t& padding) noexcept;

ErrorCode parseDataBegin(folly::io::Cursor& cursor,
                         const FrameHeader& header,
                         size_t& parsed,
                         uint16_t& outPadding) noexcept;

ErrorCode parseDataEnd(folly::io::Cursor& cursor,
                       const size_t bufLen,
                       const size_t pendingDataFramePaddingBytes,
                       size_t& toSkip) noexcept;

/**
 * This function parses the section of the HEADERS frame after the common
 * frame header. It discards any padding and returns the header data in
 * outBuf. It pulls header.length bytes from the cursor, so it is the
 * caller's responsibility to ensure there is enough data available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outPriority If PRIORITY flag is set, this will be filled with
 *                    the priority information from this frame.
 * @param outBuf The buf to fill with header data.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseHeaders(folly::io::Cursor& cursor,
                       const FrameHeader& header,
                       folly::Optional<PriorityUpdate>& outPriority,
                       std::unique_ptr<folly::IOBuf>& outBuf) noexcept;

ErrorCode parseExHeaders(folly::io::Cursor& cursor,
                         const FrameHeader& header,
                         HTTPCodec::ExAttributes& outExAttributes,
                         folly::Optional<PriorityUpdate>& outPriority,
                         std::unique_ptr<folly::IOBuf>& outBuf) noexcept;

/**
 * This function parses the section of the PRIORITY frame after the common
 * frame header. It pulls header.length bytes from the cursor, so it is the
 * caller's responsibility to ensure there is enough data available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outPriority On success, filled with the priority information
 *                    from this frame.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parsePriority(folly::io::Cursor& cursor,
                        const FrameHeader& header,
                        PriorityUpdate& outPriority) noexcept;

/**
 * This function parses the section of the RST_STREAM frame after the
 * common frame header. It pulls header.length bytes from the cursor, so
 * it is the caller's responsibility to ensure there is enough data
 * available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outCode The error code received in the frame.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseRstStream(folly::io::Cursor& cursor,
                         const FrameHeader& header,
                         ErrorCode& outCode) noexcept;

/**
 * This function parses the section of the SETTINGS frame after the
 * common frame header. It pulls header.length bytes from the cursor, so
 * it is the caller's responsibility to ensure there is enough data
 * available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param settings The settings received in this frame.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseSettings(folly::io::Cursor& cursor,
                        const FrameHeader& header,
                        std::deque<SettingPair>& settings) noexcept;

/**
 * This function parses the section of the PUSH_PROMISE frame after the
 * common frame header. It pulls header.length bytes from the cursor, so
 * it is the caller's responsibility to ensure there is enough data
 * available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outPromisedStream The id of the stream promised by the remote.
 * @param outBuf The buffer to fill with header data.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parsePushPromise(folly::io::Cursor& cursor,
                           const FrameHeader& header,
                           uint32_t& outPromisedStream,
                           std::unique_ptr<folly::IOBuf>& outBuf) noexcept;

/**
 * This function parses the section of the PING frame after the common
 * frame header. It pulls header.length bytes from the cursor, so it is
 * the caller's responsibility to ensure there is enough data available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outData The opaque data from the ping frame
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parsePing(folly::io::Cursor& cursor,
                    const FrameHeader& header,
                    uint64_t& outData) noexcept;

/**
 * This function parses the section of the GOAWAY frame after the common
 * frame header.  It pulls header.length bytes from the cursor, so
 * it is the caller's responsibility to ensure there is enough data
 * available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outLastStreamID The last stream id accepted by the remote.
 * @param outCode The error code received in the frame.
 * @param outDebugData Additional debug-data in the frame, if any
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseGoaway(folly::io::Cursor& cursor,
                      const FrameHeader& header,
                      uint32_t& outLastStreamID,
                      ErrorCode& outCode,
                      std::unique_ptr<folly::IOBuf>& outDebugData) noexcept;

/**
 * This function parses the section of the WINDOW_UPDATE frame after the
 * common frame header. The caller must ensure there is header.length
 * bytes available in the cursor.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outAmount The amount to increment the stream's window by.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseWindowUpdate(folly::io::Cursor& cursor,
                            const FrameHeader& header,
                            uint32_t& outAmount) noexcept;

/**
 * This function parses the section of the CONTINUATION frame after the
 * common frame header. The caller must ensure there is header.length
 * bytes available in the cursor.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outBuf The buffer to fill with header data.
 * @param outAmount The amount to increment the stream's window by.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseContinuation(folly::io::Cursor& cursor,
                            const FrameHeader& header,
                            std::unique_ptr<folly::IOBuf>& outBuf) noexcept;

/**
 * This function parses the section of the ALTSVC frame after the
 * common frame header. The caller must ensure there is header.length
 * bytes available in the cursor.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outMaxAge The max age field.
 * @param outPort The port the alternative service is on.
 * @param outProtocol The alternative service protocol string.
 * @param outHost The alternative service host name.
 * @param outOrigin The origin the alternative service is applicable to.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a GOAWAY frame if failure.
 */
ErrorCode parseAltSvc(folly::io::Cursor& cursor,
                      const FrameHeader& header,
                      uint32_t& outMaxAge,
                      uint32_t& outPort,
                      std::string& outProtocol,
                      std::string& outHost,
                      std::string& outOrigin) noexcept;

/**
 * This function parses the section of the CERTIFICATE_REQUEST frame after the
 * common frame header.  It pulls header.length bytes from the cursor, so it is
 * the caller's responsibility to ensure there is enough data available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outRequestId The Request-ID identifying this certificate request.
 * @param outAuthRequest Authenticator request in the frame, if any.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a CERTIFICATE_REQUEST frame if failure.
 */
ErrorCode parseCertificateRequest(
    folly::io::Cursor& cursor,
    const FrameHeader& header,
    uint16_t& outRequestId,
    std::unique_ptr<folly::IOBuf>& outAuthRequest) noexcept;

/**
 * This function parses the section of the CERTIFICATE frame after the
 * common frame header.  It pulls header.length bytes from the cursor, so it is
 * the caller's responsibility to ensure there is enough data available.
 *
 * @param cursor The cursor to pull data from.
 * @param header The frame header for the frame being parsed.
 * @param outCertId The Cert-ID identifying the frame.
 * @param outAuthenticator Authenticator fragment in the frame, if any.
 * @return NO_ERROR for successful parse. The connection error code to
 *         return in a CERTIFICATE frame if failure.
 */
ErrorCode parseCertificate(
    folly::io::Cursor& cursor,
    const FrameHeader& header,
    uint16_t& outCertId,
    std::unique_ptr<folly::IOBuf>& outAuthenticator) noexcept;

//// Egress ////

/**
 * Generate an entire DATA frame, including the common frame header.
 * The combined length of the data buffer, the padding, and the padding
 * length MUST NOT exceed 2^14 - 1, which is kMaxFramePayloadLength.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param data The body data to write out, can be nullptr for 0 length
 * @param stream The stream identifier of the DATA frame.
 * @param padding If not kNoPadding, adds 1 byte pad len and @padding pad bytes
 * @param endStream True iff this frame ends the stream.
 * @param reuseIOBufHeadroom If HTTP2Framer should reuse headroom in data if
 *                           headroom is enough for frame header
 * @return The number of bytes written to writeBuf.
 */
size_t writeData(folly::IOBufQueue& writeBuf,
                 std::unique_ptr<folly::IOBuf> data,
                 uint32_t stream,
                 folly::Optional<uint8_t> padding,
                 bool endStream,
                 bool reuseIOBufHeadroom) noexcept;

/**
 * Generate an entire HEADERS frame, including the common frame header. The
 * combined length of the data buffer and the padding and priority fields MUST
 * NOT exceed 2^14 - 1, which is kMaxFramePayloadLength.
 *
 * @param headerBuf Buffer that will contain the frame header and other fields
 *                  before the header block.  Must be sized correctly and
 *                  in the queue.  Call calculatePreHeaderBlockSize/preallocate/
 *                  postallocate.
 * @param headeBufLen Length of headerBuf
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param headersLen The length of the encoded headers data (already in writBuf)
 * @param stream The stream identifier of the HEADERS frame.
 * @param priority If present, the priority depedency information to
 *                 update the stream with.
 * @param padding If not kNoPadding, adds 1 byte pad len and @padding pad bytes
 * @param endStream True iff this frame ends the stream.
 * @param endHeaders True iff no CONTINUATION frames will follow this frame.
 * @return The number of bytes written to writeBuf.
 */
size_t writeHeaders(uint8_t* headerBuf,
                    size_t headerBufLen,
                    folly::IOBufQueue& queue,
                    size_t headersLen,
                    uint32_t stream,
                    folly::Optional<PriorityUpdate> priority,
                    folly::Optional<uint8_t> padding,
                    bool endStream,
                    bool endHeaders) noexcept;

/**
 * Generate an experimental ExHEADERS frame, including the common frame
 * header. The combined length of the data buffer and the padding and priority
 * fields MUST NOT exceed 2^14 - 1, which is kMaxFramePayloadLength.
 *
 * @param headerBuf Buffer that will contain the frame header and other fields
 *                  before the header block.  Must be sized correctly and
 *                  in the queue.  Call calculatePreHeaderBlockSize/preallocate/
 *                  postallocate.
 * @param headeBufLen Length of headerBuf
 * @param queue The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param headersLen The length encoded headers (already in queue).
 * @param stream The stream identifier of the ExHEADERS frame.
 * @param exAttributes Attributes specific to ExHEADERS frame.
 * @param priority If present, the priority depedency information to
 *                 update the stream with.
 * @param padding If not kNoPadding, adds 1 byte pad len and @padding pad bytes
 * @param endStream True iff this frame ends the stream.
 * @param endHeaders True iff no CONTINUATION frames will follow this frame.
 * @return The number of bytes written to writeBuf.
 */
size_t writeExHeaders(uint8_t* headerBuf,
                      size_t headerBufLen,
                      folly::IOBufQueue& queue,
                      size_t headersLen,
                      uint32_t stream,
                      const HTTPCodec::ExAttributes& exAttributes,
                      const folly::Optional<PriorityUpdate>& priority,
                      const folly::Optional<uint8_t>& padding,
                      bool endStream,
                      bool endHeaders) noexcept;

/**
 * Generate an entire PRIORITY frame, including the common frame header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param stream The stream identifier of the DATA frame.
 * @param priority The priority depedency information to update the stream with.
 * @return The number of bytes written to writeBuf.
 */
size_t writePriority(folly::IOBufQueue& writeBuf,
                     uint32_t stream,
                     PriorityUpdate priority) noexcept;

/**
 * Generate an entire RST_STREAM frame, including the common frame
 * header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param stream The identifier of the stream to reset.
 * @param errorCode The error code returned in the frame.
 * @return The number of bytes written to writeBuf.
 */
size_t writeRstStream(folly::IOBufQueue& writeBuf,
                      uint32_t stream,
                      ErrorCode errorCode) noexcept;

/**
 * Generate an entire SETTINGS frame, including the common frame
 * header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param settings The settings to send
 * @return The number of bytes written to writeBuf.
 */
size_t writeSettings(folly::IOBufQueue& writeBuf,
                     const std::deque<SettingPair>& settings);

/**
 * Writes an entire empty SETTINGS frame, including the common frame
 * header. No settings can be transmitted with this frame.
 */
size_t writeSettingsAck(folly::IOBufQueue& writeBuf);

/**
 * Writes an entire PUSH_PROMISE frame, including the common frame
 * header.
 *
 * @param headerBuf Buffer that will contain the frame header and other fields
 *                  before the header block.  Must be sized correctly and
 *                  in the queue.  Call calculatePreHeaderBlockSize/preallocate/
 *                  postallocate.
 * @param headeBufLen Length of headerBuf
 * @param queue The output queue to write to. It may grow or add
 *              underlying buffers inside this function.
 * @param associatedStream The identifier of the stream the promised
 *                         stream is associated with.
 * @param promisedStream The identifier of the promised stream.
 * @param headersLen The length of the encoded headers (already in queue).
 * @param padding If not kNoPadding, adds 1 byte pad len and @padding pad bytes
 * @param endHeaders True iff no CONTINUATION frames will follow this frame.
 * @return The number of bytes written to writeBuf/
 */
size_t writePushPromise(uint8_t* headerBuf,
                        size_t headerBufLen,
                        folly::IOBufQueue& queue,
                        uint32_t associatedStream,
                        uint32_t promisedStream,
                        size_t headersLen,
                        folly::Optional<uint8_t> padding,
                        bool endHeaders) noexcept;

/**
 * Generate an entire PING frame, including the common frame header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param data The opaque data to include.
 * @param ack True iff this is a ping response.
 * @return The number of bytes written to writeBuf.
 */
size_t writePing(folly::IOBufQueue& writeBuf, uint64_t data, bool ack) noexcept;

/**
 * Generate an entire GOAWAY frame, including the common frame
 * header. We do not implement the optional opaque data.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param lastStreamID The identifier of the last stream accepted.
 * @param errorCode The error code returned in the frame.
 * @param debugData Optional debug information to add to the frame
 * @return The number of bytes written to writeBuf.
 */
size_t writeGoaway(folly::IOBufQueue& writeBuf,
                   uint32_t lastStreamID,
                   ErrorCode errorCode,
                   std::unique_ptr<folly::IOBuf> debugData = nullptr) noexcept;

/**
 * Generate an entire WINDOW_UPDATE frame, including the common frame
 * header. |amount| MUST be between 1 to 2^31 - 1 inclusive
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param stream The stream to send a WINDOW_UPDATE on
 * @param amount The number of bytes to AK
 * @return The number of bytes written to writeBuf.
 */
size_t writeWindowUpdate(folly::IOBufQueue& writeBuf,
                         uint32_t stream,
                         uint32_t amount) noexcept;

/**
 * Generate an entire CONTINUATION frame, including the common frame
 * header. The combined length of the data buffer and the padding MUST NOT
 * exceed 2^14 - 3, which is kMaxFramePayloadLength minus the two bytes to
 * encode the length of the padding.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param stream The stream identifier of the DATA frame.
 * @param endHeaders True iff more CONTINUATION frames will follow.
 * @param headers The encoded headers data to write out.
 * @return The number of bytes written to writeBuf.
 */
size_t writeContinuation(folly::IOBufQueue& queue,
                         uint32_t stream,
                         bool endHeaders,
                         std::unique_ptr<folly::IOBuf> headers) noexcept;
/**
 * Generate an entire ALTSVC frame, including the common frame
 * header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param stream The stream to do Alt-Svc on. May be zero.
 * @param maxAge The max age field.
 * @param port The port the alternative service is on.
 * @param protocol The alternative service protocol string.
 * @param host The alternative service host name.
 * @param origin The origin the alternative service is applicable to.
 * @return The number of bytes written to writeBuf.
 */
size_t writeAltSvc(folly::IOBufQueue& writeBuf,
                   uint32_t stream,
                   uint32_t maxAge,
                   uint16_t port,
                   folly::StringPiece protocol,
                   folly::StringPiece host,
                   folly::StringPiece origin) noexcept;

/**
 * Generate an entire CERTIFICATE_REQUEST frame, including the common frame
 * header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param requestId The opaque Request-ID of this used to correlate subsequent
 *                  certificate-related frames with this request.
 * @param authRequest The encoded authenticator request.
 * @return The number of bytes written to writeBuf.
 */
size_t writeCertificateRequest(folly::IOBufQueue& writeBuf,
                               uint16_t requestId,
                               std::unique_ptr<folly::IOBuf> authRequest);

/**
 * Generate an entire CERTIFICATE frame, including the common frame
 * header.
 *
 * @param writeBuf The output queue to write to. It may grow or add
 *                 underlying buffers inside this function.
 * @param certId The opaque Cert-ID of this frame which is used to correlate
 * subsequent certificate-related frames with this certificate.
 * @param authenticator The encoded authenticator fragment.
 * @param toBeContinued Indicates whether there is additional authenticator
 * fragment.
 * @return The number of bytes written to writeBuf.
 */
size_t writeCertificate(folly::IOBufQueue& writeBuf,
                        uint16_t certId,
                        std::unique_ptr<folly::IOBuf> authenticator,
                        bool toBeContinued);

/**
 * Get the string representation of the given FrameType
 *
 * @param type frame type
 *
 * @return string representation of the frame type
 */
const char* getFrameTypeString(FrameType type);

/**
 * Calculate the amount of space needed for the frame header and any payload
 * components that come before the header block.
 *
 * @param hasAssociatedStream Set for PUSH_PROMISE
 * @param hasExAttributes Set for EX_HEADERS
 * @param hasPriority Set if there is priority
 * @param hasPadding Set if there is padding
 */
uint8_t calculatePreHeaderBlockSize(bool hasAssocStream,
                                    bool hasExAttributes,
                                    bool hasPriority,
                                    bool hasPadding);

}} // namespace proxygen::http2
