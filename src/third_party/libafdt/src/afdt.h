/**
   @mainpage

   @section intro Introduction

   libafdt is a library for "a"synchronous "f"ile "d"escriptor "t"ransfers.
   It provides a simple interface that allows libevent-based programs to
   set up a Unix domain socket to accept connections and transfer file
   descriptors to clients, or to be a client and request a file descriptor
   from a libafdt server.  Low-level and synchronous interfaces are also
   provided for programs that do not use libevent.

   See <em>Advanced Programming in the UNIX(R) Environment</em> for more
   information about transferring file descriptors between processes using
   Unix domain sockets.

   @section structure Structure

   libafdt's has two high-level interfaces: an \link async Asynchronous
   Interface \endlink and a \link sync Synchronous Interface \endlink .
   The former is best for clients or servers that are based on libevent.
   It uses callbacks to allow the program to process high-level events.
   The latter is for clients that don't use libevent and are willing to
   block while waiting to receive a response.  It does not require
   libevent, and is much more convenient when blocking is acceptable.

   There is also a \link lowlevel Low-level Interface \endlink .  It does
   not require libevent, but can still be used without blocking.  It can
   also be used to change the simple protocol used by the high-level
   interfaces.  It requires a bit more bookkeeping to use.

   @section examples Examples

   The test programs are good examples of how to use the high-level
   interfaces.

   The high-level interfaces are good examples of how to use the low-level
   interface.
 */

/**
   @file
   @brief  The main header file for libafdt
  
   This file contains all of the types and functions available in
   the libafdt API.  All other files are for internal use only.
 */

#ifndef _AFDT_H
#define _AFDT_H

// Put this in an ifndef so it doesn't go into the generated man page.
#ifndef DOXYGEN
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief  Maximum request or response message length, including the header.
 *
 * This is relatively small so that we can consider short reads writes to be
 * exceptional circumstances, rather than business as usual.
 */
#define AFDT_FULL_MSGLEN 0x200

/**
 * @brief  Maximum request or response message length, excluding the header.
 */
#define AFDT_MSGLEN (AFDT_FULL_MSGLEN - sizeof(uint32_t))


// Declare these as incomplete types.
// The appropriate headers need to be included in order to use them.
struct event_base;
struct timeval;


/**
   @defgroup errors Error Handling
  
   Types, values, and functions used for reporting errors.
 */

/**
 * @ingroup errors
 * @brief  Phase of the AFDT process during which an error occurred.
 *
 * These error codes are used by the asynchronous interface to indicate
 * the high-level operation that triggered an error.
 */
enum afdt_phase {
  AFDT_NO_PHASE,
  AFDT_CREATE_SERVER,
  AFDT_ACCEPT_CLIENT,
  AFDT_HANDLE_REQUEST,
  AFDT_CREATE_CLIENT,
  AFDT_HANDLE_RESPONSE,
};

/**
 * @ingroup errors
 * @brief  Operation that resulted in an error.
 *
 * These error codes are used by the all interfaces to indicate the low-level
 * operation that triggered an error.  These are mostly system calls.
 */
enum afdt_operation {
  AFDT_NO_OPERATION,
  AFDT_MALLOC,
  AFDT_SOCKET,
  AFDT_PATHNAME,
  AFDT_BIND,
  AFDT_LISTEN,
  AFDT_ACCEPT,
  AFDT_CONNECT,
  AFDT_FORMAT,
  AFDT_SENDMSG,
  AFDT_RECVMSG,
  AFDT_EVENT_BASE_SET,
  AFDT_EVENT_ADD,
  AFDT_POLL,
  AFDT_TIMEOUT,
};

/**
 * @ingroup errors
 * @brief  Detailed information about an error.
 */
struct afdt_error_t {
  /// @brief  High-level phase during which the error occurred.
  enum afdt_phase phase;
  /// @brief  Low-level operation that caused the error.
  enum afdt_operation operation;
  /// @brief  Additional human-readable information, or "".
  const char* message;
};

/**
 * @ingroup errors
 * @brief  Initializer for error information.
 *
 * All \c afdt_error_t objects passed to functions in the low-level interface
 * should be initialized with this value.
 */
extern const struct afdt_error_t AFDT_ERROR_T_INIT;

/**
 * @ingroup errors
 * @brief  Convert \c afdt_phase into a textual description.
 */
const char* afdt_phase_str(enum afdt_phase phase);

/**
 * @ingroup errors
 * @brief  Convert \c afdt_operation into a textual description.
 */
const char* afdt_operation_str(enum afdt_operation operation);


/**
   @defgroup lowlevel Low-level Interface
  
   The low-level interface provides a thin abstraction over the
   primitives used for transferring file descriptors over Unix domain
   sockets.  The higher-level interfaces are implemented in terms of
   these functions, which can also be used to implement a high-level
   interface on top of a different event loop API or a high-level
   interface obeying a different protocol (for example: the client
   sending the file descriptor to the server).

   This layer imposes very few restrictions on usage.  Messages must
   be at most AFDT_MSGLEN bytes, and are automatically prefixed with
   a 32-bit host-byte-order header equal to the message length.
   At most one one file descriptor can be passed per message.

   The most interesting operation provided by this interface is
   send/recv fd_msg, which is a short message and an optional
   file descriptor.  Functions are also provided for sending and
   receiving "plain" messages, which do not include file descriptors.

   All functions in this interface report errors by returning a negative
   value and populating detailed information into their \a err parameter.
   (\a err should be initialized to \c AFDT_ERROR_T_INIT.)  errno will
   be set to an appropriate code, or 0 if no error code is applicable.
 */

/**
 * @ingroup lowlevel
 * @brief  Create a socket that listens for connections.
 *
 * Higher level code should call \c accept(2) on the returned socket
 * to accept a connection from a client.
 *
 * @param fname  File to bind PF_LOCAL socket to.
 * @param err    Structure to populate with error information.
 *
 * @return  socket fd (>=0) if successful, <0 on failure.
 */
int afdt_listen(const char* fname, struct afdt_error_t* err);

/**
 * @ingroup lowlevel
 * @brief  Create a socket and connect to a listening socket.
 *
 * @param fname  File to connect PF_LOCAL socket to.
 * @param err    Structure to populate with error information.
 *
 * @return  socket fd (>=0) if successful, <0 on failure.
 */
int afdt_connect(const char* fname, struct afdt_error_t* err);

/**
 * @ingroup lowlevel
 * @brief  Send a message with an optional file descriptor.
 *
 * @param connfd       Descriptor from \c accept or \c afdt_connect.
 * @param content      Message content.
 * @param content_len  Message length.
 * @param fd_to_send   File descriptor to send, or <0 for none.
 * @param err          Structure to populate with error information.
 *
 * @return  >=0 if successful, <0 on failure.
 */
int afdt_send_fd_msg(
    int connfd, 
    const uint8_t* content,
    uint32_t content_len,
    int fd_to_send,
    struct afdt_error_t* err);

/**
 * @ingroup lowlevel
 * @brief  Send a message with no file descriptor.
 * @see afdt_send_fd_msg
 */
int afdt_send_plain_msg(
    int connfd,
    const uint8_t* content,
    uint32_t content_len,
    struct afdt_error_t* err);

/**
 * @ingroup lowlevel
 * @brief  Receive a message with an optional file descriptor.
 *
 * @param connfd       Descriptor from \c accept or \c afdt_connect.
 * @param content      Buffer for message content.
 * @param content_len  Pointer to buffer length, returns with actual length.
 * @param received_fd  Returns with received fd, or <0 if none.
 * @param err          Structure to populate with error information.
 *
 * @return  >=0 if successful, <0 on failure.
 */
int afdt_recv_fd_msg(
    int connfd,
    uint8_t* content,
    uint32_t* content_len,
    int *received_fd,
    struct afdt_error_t* err);

/**
 * @ingroup lowlevel
 * @brief  Receive a message with no file descriptor.
 * @see afdt_recv_fd_msg
 */
int afdt_recv_plain_msg(
    int connfd,
    uint8_t* content,
    uint32_t* content_len,
    struct afdt_error_t* err);


/**
   @defgroup async Asynchronous Interface (libevent-based)
  
   The asynchronous interface provides functions for setting up clients
   and servers that use event-driven I/O to avoid blocking a thread.
   libevent is used to make the interface work with many different
   event APIs.

   The asynchronous interface is adds more restrictions than the
   low-level API.  Interactions are always composed of two messages:
   a plain message from the client to the server (the request) followed
   by an fd message from the server to the client(the response).
   There is currently no support for a client sending a file descriptor
   to the server.  (In most cases, it is not too hard to invert the
   "client" and "server" roles to get around this.  The server is simply
   the program that listens on the Unix domain socket.)

   Most functions in this interface report errors by calling an error
   callback and passing error information to it.  errno will be set to
   an appropriate code, or 0 if no error code is applicable.
 */

/**
 * @ingroup async
 * @brief  Callback type for processing an AFDT request.
 *
 * For example, a callback could check some authentication information
 * in the request, look in the request for the name of some resource,
 * then return the appropriate fd or send an error message in the response.
 * Remember to set *\a response_length to 0 if you are sending an empty
 * response.
 *
 * @param request          Request buffer sent by the client.
 * @param request_length   Length of \a request.
 * @param response         Buffer into which to write the response.
 * @param response_length  IN/OUT: \a response buffer/content size.
 * @param userdata         Extra parameter passed to \c create_server.
 * @return  File descriptor to send, or <0 for none.
 */
typedef int (*afdt_request_handler_t)(
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    void *userdata);

/**
 * @ingroup async
 * @brief  Callback type for processing an AFDT response.
 *
 * For example, a callback could pass the file descriptor to
 * \e evhttp_accept_socket and start serving requests, or send another
 * request if the response indicates that the transfer was not authorized.
 *
 * @param response         Response buffer sent by the server.
 * @param response_length  Length of \a response.
 * @param received_fd      File descriptor received, or <0 for none.
 * @param userdata         Extra parameter passed to \c create_client.
 */
typedef void (*afdt_response_handler_t)(
    const uint8_t* response,
    uint32_t response_length,
    int received_fd,
    void *userdata);

/**
 * @ingroup async
 * @brief  Callback type for after a response is sent.
 *
 * For example, a callback could close \a sent_fd if the recipient is
 * going to take over all operations.
 *
 * @param request          Request buffer sent by the client.
 * @param request_length   Length of \a request.
 * @param response         Response buffer sent by the server.
 * @param response_length  Length of \a response.
 * @param received_fd      File descriptor sent (by \a request_handler).
 * @param userdata         Extra parameter passed to \c create_server.
 */
typedef void (*afdt_post_handler_t)(
    const uint8_t* request,
    uint32_t request_length,
    const uint8_t* response,
    uint32_t response_length,
    int sent_fd,
    void *userdata);

/**
 * @ingroup async
 * @brief  Callback type for processing errors during AFDT operations.
 *
 * For example, a callback could log an error message, then try some
 * other means of acquiring the resources it needs.
 *
 * @param phase      AFDT phase during which the error occurred.
 * @param operation  Operation that resulted in the error.
 * @param message    Extra information message (often empty).
 * @param userdata   Extra parameter passed to
 *                   \c create_client or \c create_server.
 */
typedef void (*afdt_error_handler_t)(
    const struct afdt_error_t* err,
    void *userdata);

/**
 * @ingroup async
 * @brief  No-op function that can be used as a post_handler.
 */
void afdt_no_post(
    const uint8_t* request,
    uint32_t request_length,
    const uint8_t* response,
    uint32_t response_length,
    int sent_fd,
    void *userdata);

/**
 * @ingroup async
 * @brief  Create a server to make file descriptors available.
 *
 * \a request_handler and \a error_handler may both be called
 * multiple times.
 *
 * @param fname             File to bind PF_LOCAL socket to.
 *                          Must not exist.
 * @param eb                struct event_base to manage this server.
 * @param request_handler   Callback to handle requests.
 * @param post_handler      Callback called after sending a response.
 * @param error_handler     Callback to handle errors.
 * @param out_close_handle  If non-\c NULL, set to a handle for
 *                          \c afdt_close_server.
 * @param userdata          Arbitrary value to pass to callbacks.
 * @return  >=0 if successful, <0 on failure.
 *          \a error_handler will also be called on failure.
 */
int afdt_create_server(
    const char* fname,
    struct event_base* eb,
    afdt_request_handler_t request_handler,
    afdt_post_handler_t post_handler,
    afdt_error_handler_t error_handler,
    void** out_close_handle,
    void* userdata);

/**
 * @ingroup async
 * @brief  Shut down a server created by \c afdt_create_server.
 *
 * Any client connections that have already been created
 * will continue to exist.
 *
 * @param close_handle  Handle provided by \c afdt_create_server.
 * @return  >=0 if successful, <0 on failure.
 */
int afdt_close_server(void* close_handle);

/**
 * @ingroup async
 * @brief  Request a file descriptor from a server.
 *
 * Exactly one of \a response_handler and \a error_handler will be called,
 * unless timeout is \c NULL and the server never responds.
 *
 * \note  This function currently does a blocking connect and send.
 *
 * @param fname             File to connect PF_LOCAL socket to.
 * @param eb                struct event_base to manage this client.
 * @param request           Request buffer to send.
 * @param request_length    Length of \a request.
 * @param response_handler  Callback to handle response.
 * @param error_handler     Callback to handle errors.
 * @param timeout           Timeout when waiting for response, or \c NULL.
 * @param userdata          Arbitrary value to pass to callbacks.
 * @return  >=0 if connection and request successful, <0 on failure.
 *          \a error_handler will also be called on failure.
 */
int afdt_create_client(
    const char* fname,
    struct event_base* eb,
    const uint8_t* request,
    uint32_t request_length,
    afdt_response_handler_t response_handler,
    afdt_error_handler_t error_handler,
    const struct timeval* timeout,
    void* userdata);


/**
   @defgroup sync Synchronous Client Interface
  
   The synchronous interface provides a very simple function allowing
   clients to communicate with servers using the asynchronous interface.
   The function is blocking, but supports an optional timeout.
 */

/**
 * @ingroup sync
 * @brief  Request a file descriptor from a server.
 *
 * @param fname             File to connect PF_LOCAL socket to.
 * @param request           Request buffer to send.
 * @param request_length    Length of \a request.
 * @param response          Buffer for response.
 * @param response_length   Pointer to \a response length,
 *                          returns with actual length.
 * @param received_fd       Returns with received fd, or <0 if none.
 * @param timeout           Timeout when waiting for response, or \c NULL.
 * @param err    Structure to populate with error information.
 * @return  >=0 if successful, <0 on failure.
 */
int afdt_sync_client(
    const char* fname,
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    int* received_fd,
    const struct timeval* timeout,
    struct afdt_error_t* err);

#ifdef __cplusplus
}
#endif

#endif // _AFDT_H
