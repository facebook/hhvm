/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_PROTOCOL_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_PROTOCOL_H_

#include <folly/Bits.h>

namespace HPHP {
namespace fcgi {

namespace detail {
template<typename T>
struct wire {
  wire() = default;
  /* implicit */ wire(T s) : raw(folly::Endian::big(s)) {}
  wire& operator=(T s) { raw = folly::Endian::big(s); return *this; }
  /* implicit */ operator T() const { return folly::Endian::big(raw); }

private:
  T raw;
} __attribute__((__packed__));
}

/* FastCGI sends all data over the wire in big-endian format. These helper types
 * marshal data within records for transmition.
 */
using wshort = detail::wire<uint16_t>;
using wlong  = detail::wire<uint32_t>;
using wbyte  = uint8_t;

enum class Version : uint8_t {
  Current = 1
};

/* FastCGI Records fall into two three categories:
 *
 * Structured records contain a fixed contentData structure with values at
 * specific offsets and numeric data encoded in big-endian form.
 *
 * Unstructured streams contain raw bytes to be processed by the web and
 * application servers (e.g. standard input).
 *
 * Structured streams contain name-value pairs encoded as a length followed
 * by an ascii string. Numeric values are encoded as strings.
 *
 * Streams of the same type are to be concatentated together.
 */
enum Type : uint8_t {
  /* FastCGI BEGIN_REQUEST intiates a new http request. The contentData for
   * such records is structured as follows-
   *
   * uint16: role
   * uint8 : flags
   * XXXXX : reserved (5 bytes)
   *
   * The roles are outlined below and indicate the nature of the handling
   * required of the application server. We only support the RESPONDER role.
   */
  BEGIN_REQUEST = 1, // [in]

  /* FastCGI ABORT_REQUEST prematurely terminates a request, signaling that
   * the webserver is no longer interested in processing it. The contentData
   * for such recoreds is empty. The application server must reply with
   * an END_REQUEST record to indicate that it has aborted.
   */
  ABORT_REQUEST = 2, // [in]

  /* FastCGI END_REQUEST is sent to the webserver to mark the completion of
   * a request it contains the exit code for the request as well as the a
   * protocol defined status field indicating the reason the request was
   * terminated, the contentData is structured as follows-
   *
   * uint32: appStatus (the exit code)
   * uint8 : protocolStatus (the reason)
   * XXXXX : reserved (3 bytes)
   */
  END_REQUEST = 3, // [out]

  /* FastCGI PARAMS is a /stream/ record consisting of name-value pairs
   * which inform the application server of request parameters.
   */
  PARAMS = 4, // [in]

  /* FastCGI STDIN, STDOUT, STDERR, and DATA are /stream/ records containing
   * unstructured data.
   *
   * STDIN- used as standard input by the application server.
   * STDOUT- standard output from the application server to be processed by
   *         the webserver.
   * STDERR- standard error from the application server to be processed by
   *         the webserver.
   * DATA- additional data sent from the webserver to the application to be
   *       processed.
   */
  STDIN = 5,  // [in]  POST data
  STDOUT = 6, // [out] response
  STDERR = 7, // [out] errors
  DATA = 8,   // [in]  (unsupported)

  /* FastCGI GET_VALUES and GET_VALUES_RESULT are used to communicate the
   * capabilities of the application server to the webserver.
   *
   * - These records should be associated with the null reqeuestId.
   * - The contentData of GET_VALUES contains name-value pairs with only
   *   names, while the contentData of GET_VALUES_RESULT contains name-value
   *   pairs containing the values of the requested parameters.
   *
   * FCGI_MAX_CONNS- maximum number of concurrent connections
   * FCGI_MAX_REQS- maximum number of concurrent requests
   * FCGI_MPXS_CONNS- are multiplexed connections supported
   *
   * The values are all numeric but should be encoded as ascii strings.
   */
  GET_VALUES = 9,         // [in]
  GET_VALUES_RESULT = 10, // [out]

  /* FastCGI UNKNOWN_TYPE is sent from an application that has received a
   * record it does not understand. The record is structured as follows-
   *
   * uint8 : type (the record type received)
   * XXXXX : reserved (7 bytes)
   */
  UNKNOWN_TYPE = 11 // [out]
};

enum Flags : uint8_t {
  KEEP_CONN = 1 // connection should remain open
};

enum Role : uint16_t {
  RESPONDER = 1,  // send response for http request
  AUTHORIZER = 2, // (unsupported) decide if request should be accepted
  FILTER = 3      // (unsupported) rewrites request stream
};

enum Status : uint8_t {
  REQUEST_COMPLETE = 0,    // completed processing request
  CANT_MULTIPLEX_CONN = 1, // already processing a different request
  OVERLOADED = 2,          // cannot process a new request
  UNKNOWN_ROLE = 3         // cannot perform requested role
};

////////////////////////////////////////////////////////////////////////////////

/* FastCGI Records have a common header, all multibyte values are big endian:
 *
 * uint8 : version
 * uint8 : type
 * uint16: requestId
 * uint16: contentLength (between 0 and 65535)
 * uint8 : paddingLength (between 0 and 255)
 * uint8 : reserved
 * XXXXX : contentData (contentLength many bytes)
 * XXXXX : paddingData (paddingLength many bytes)
 *
 * The sender is not required to send padding bytes, the receiver must ignore
 * them if sent.
 */
struct record {
  Version version;
  Type type;

  wshort requestId;
  wshort contentLength;
  wbyte paddingLength;
  wbyte reserved;

  size_t size() const {
    return sizeof(record) + (uint16_t)contentLength + paddingLength;
  }

  const uint8_t* data() const {
    return reinterpret_cast<const uint8_t*>(this + 1);
  }

  const uint8_t* pad()  const { return data() + (uint16_t)contentLength; }

  template<typename T>
  typename std::enable_if<
    std::is_base_of<record, T>::value,
    T*
  >::type getTyped() {
    return reinterpret_cast<T*>(this);
  }

  template<typename T>
  typename std::enable_if<
    std::is_base_of<record, T>::value,
    const T*
  >::type getTyped() const {
    return reinterpret_cast<const T*>(this);
  }
} __attribute__((__packed__));


struct begin_record : public record {
  wshort role;
  Flags flags;
  uint8_t reserved[5];

  bool keepAlive() { return flags & KEEP_CONN; }
} __attribute__((__packed__));

struct abort_record : public record {} __attribute__((__packed__));

struct end_record : public record {
  wlong appStatus;
  Status protStatus;
  uint8_t reserved[3];
} __attribute__((__packed__));

struct unknown_record : public record {
  Type unknownType;
  uint8_t reserved[7];
} __attribute__((__packed__));

#define STREAM_RECORD(name) \
  struct name : public record {} __attribute__((__packed__))

STREAM_RECORD(stdin_record);
STREAM_RECORD(stdout_record);
STREAM_RECORD(data_record);
STREAM_RECORD(params_record);
STREAM_RECORD(values_record);
STREAM_RECORD(values_result_record);

#undef STREAM_RECORD

////////////////////////////////////////////////////////////////////////////////
}}
#endif
