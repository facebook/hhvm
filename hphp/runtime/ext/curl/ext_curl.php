<?hh

namespace {

/**
 * Close a cURL session
 *
 * @param resource $ch -
 *
 * @return void -
 */
<<__Native>>
function curl_close(resource $ch)[leak_safe]: ?bool;

/**
 * Return the last error number
 *
 * @param resource $ch -
 *
 * @return int - Returns the error number or 0 (zero) if no error
 *   occurred.
 */
<<__Native>>
function curl_errno(resource $ch): mixed;

/**
 * Return a string containing the last error for the current session
 *
 * @param resource $ch -
 *
 * @return string - Returns the error message or '' (the empty string) if
 *   no error occurred.
 */
<<__Native>>
function curl_error(resource $ch)[]: mixed;

/**
 * Returns a string description of a cURL error code
 *
 * @param int $errno - a curl error code, e.g. returned by curl_errno()
 *
 * @return string - Returns a string description of a cURL error code
 */
<<__Native>>
function curl_strerror(int $errno): string;

/**
 * Perform a cURL session
 *
 * @param resource $ch -
 *
 * @return mixed - However, if the CURLOPT_RETURNTRANSFER option is set,
 *   it will return the result on success, FALSE on failure.
 *
 * FCallBuiltin is not used, as it would optimize away event hooks, resulting
 * in broken request timeout semantics. It's also desirable to make curl_exec()
 * frames visible in profiling tools such as Xenon.
 */
<<__Native("NoFCallBuiltin")>>
function curl_exec(resource $ch): mixed;

/**
 * Get information regarding a specific transfer
 *
 * @param resource $ch -
 * @param int $opt - This may be one of the following constants:
 *   CURLINFO_EFFECTIVE_URL - Last effective URL     CURLINFO_HTTP_CODE -
 *   Last received HTTP code     CURLINFO_FILETIME - Remote time of the
 *   retrieved document, if -1 is returned the time of the document is
 *   unknown     CURLINFO_TOTAL_TIME - Total transaction time in seconds
 *   for last transfer     CURLINFO_NAMELOOKUP_TIME - Time in seconds until
 *   name resolving was complete     CURLINFO_CONNECT_TIME - Time in
 *   seconds it took to establish the connection
 *   CURLINFO_PRETRANSFER_TIME - Time in seconds from start until just
 *   before file transfer begins     CURLINFO_STARTTRANSFER_TIME - Time in
 *   seconds until the first byte is about to be transferred
 *   CURLINFO_REDIRECT_COUNT - Number of redirects
 *   CURLINFO_REDIRECT_TIME - Time in seconds of all redirection steps
 *   before final transaction was started     CURLINFO_SIZE_UPLOAD - Total
 *   number of bytes uploaded     CURLINFO_SIZE_DOWNLOAD - Total number of
 *   bytes downloaded     CURLINFO_SPEED_DOWNLOAD - Average download speed
 *      CURLINFO_SPEED_UPLOAD - Average upload speed
 *   CURLINFO_HEADER_SIZE - Total size of all headers received
 *   CURLINFO_HEADER_OUT - The request string sent. For this to work, add
 *   the CURLINFO_HEADER_OUT option to the handle by calling curl_setopt()
 *      CURLINFO_REQUEST_SIZE - Total size of issued requests, currently
 *   only for HTTP requests     CURLINFO_SSL_VERIFYRESULT - Result of SSL
 *   certification verification requested by setting CURLOPT_SSL_VERIFYPEER
 *       CURLINFO_CONTENT_LENGTH_DOWNLOAD - content-length of download,
 *   read from Content-Length: field     CURLINFO_CONTENT_LENGTH_UPLOAD -
 *   Specified size of upload     CURLINFO_CONTENT_TYPE - Content-Type: of
 *   the requested document, NULL indicates server did not send valid
 *   Content-Type: header
 *
 * @return mixed - If opt is given, returns its value. Otherwise, returns
 *   an associative array with the following elements (which correspond to
 *   opt), or FALSE on failure:    "url"     "content_type"     "http_code"
 *       "header_size"     "request_size"     "filetime"
 *   "ssl_verify_result"     "redirect_count"     "total_time"
 *   "namelookup_time"     "connect_time"     "pretransfer_time"
 *   "size_upload"     "size_download"     "speed_download"
 *   "speed_upload"     "download_content_length"
 *   "upload_content_length"     "starttransfer_time"     "redirect_time"
 *     "certinfo"     "request_header" (This is only set if the
 *   CURLINFO_HEADER_OUT is set by a previous call to curl_setopt())
 */
<<__Native>>
function curl_getinfo(resource $ch,
                      int $opt = 0): mixed;

/**
 * Initialize a cURL session
 *
 * @param string $url - If provided, the CURLOPT_URL option will be set
 *   to its value. You can manually set this using the curl_setopt()
 *   function.    The file protocol is disabled by cURL if open_basedir is
 *   set.
 *
 * @return resource - Returns a cURL handle on success, FALSE on errors.
 */
<<__Native>>
function curl_init(?string $url = null)[leak_safe]: mixed;

/**
 * Add a normal cURL handle to a cURL multi handle
 *
 * @param resource $mh -
 * @param resource $ch -
 *
 * @return int - Returns 0 on success, or one of the CURLM_XXX errors
 *   code.
 */
<<__Native>>
function curl_multi_add_handle(resource $mh,
                               resource $ch): ?int;

/**
 * Close a set of cURL handles
 *
 * @param resource $mh -
 *
 * @return void -
 */
<<__Native>>
function curl_multi_close(resource $mh): mixed;

/**
 * Run the sub-connections of the current cURL handle
 *
 * @param resource $mh -
 * @param int $still_running - A reference to the number of transfers
 *   that are still running
 *
 * @return int - A cURL code defined in the cURL Predefined Constants.
 *   This only returns errors regarding the whole multi stack. There might
 *   still have occurred problems on individual transfers even when this
 *   function returns CURLM_OK.
 *
 * See curl_exec() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function curl_multi_exec(resource $mh, inout int $still_running): ?int;

/**
 * Return the content of a cURL handle if  is set
 *
 * @param resource $ch -
 *
 * @return string - Return the content of a cURL handle if
 *   CURLOPT_RETURNTRANSFER is set.
 */
<<__Native>>
function curl_multi_getcontent(resource $ch): ?string;

/**
 * Get information about the current transfers
 *
 * @param resource $mh -
 * @param int $msgs_in_queue - Number of messages that are still in the
 *   queue
 *
 * @return array - On success, returns an associative array for the
 *   message, FALSE on failure.    Contents of the returned array    Key:
 *   Value:     msg The CURLMSG_DONE constant. Other return values are
 *   currently not available.   result One of the CURLE_* constants. If
 *   everything is OK, the CURLE_OK will be the result.   handle Resource
 *   of type curl indicates the handle which it concerns.
 */
<<__Native>>
function curl_multi_info_read(resource $mh,
                              <<__OutOnly("KindOfInt64")>>
                              inout mixed $msgs_in_queue): mixed;

/**
 * Returns a new cURL multi handle
 *
 * @return resource - Returns a cURL multi handle resource on success,
 *   FALSE on failure.
 */
<<__Native>>
function curl_multi_init(): resource;

/**
 * Returns a text error message describing the given CURLM error code.
 *
 * @return string - Returns error string for valid error code,
 *   NULL otherwise.
 */
<<__Native>>
function curl_multi_strerror(int $errornum): mixed;

/**
 * Remove a multi handle from a set of cURL handles
 *
 * @param resource $mh -
 * @param resource $ch -
 *
 * @return int - Returns 0 on success, or one of the CURLM_XXX error
 *   codes.
 */
<<__Native>>
function curl_multi_remove_handle(resource $mh,
                                  resource $ch): ?int;

/**
 * Wait for activity on any curl_multi connection
 *
 * @param resource $mh -
 * @param float $timeout - Time, in seconds, to wait for a response.
 *
 * @return int - On success, returns the number of descriptors contained
 *   in the descriptor sets. On failure, this function will return -1 on a
 *   select failure or timeout (from the underlying select system call).
 *
 * See curl_exec() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function curl_multi_select(resource $mh,
                           float $timeout = 1.0): ?int;

/**
 * The async equivalent to
 * [`curl_multi_select`](http://php.net/manual/en/function.curl-multi-select.php)
 *
 * This function waits until there is activity on a cURL handle within `$mh`.
 * Once there is activity, you process the result with
 * [`curl_multi_exec`](http://php.net/manual/en/function.curl-multi-exec.php)
 *
 * @param $mh - A cURL multi handle returned from
 *              [`curl_multi_init`](http://php.net/manual/en/function.curl-multi-init.php).
 * @param $timeout - The time to wait for a response indicating some activity.
 *
 * @return Awaitable<int> - An `Awaitable` representing the `int` result of the
 *                          activity. If returned `int` is positive, that
 *                          represents the number of handles on which there
 *                          was activity. If `0`, that means no activity
 *                          occurred. If negative, then there was a select
 *                          failure.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 *
 * See curl_exec() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function curl_multi_await(resource $mh,
                          float $timeout = 1.0): Awaitable<int>;

/**
 * Wait for activity on any curl_multi connection
 *
 * @param resource $mh -
 * @param int $option - One of the CURLMOPT_* constants.
 * @param int $option - The value to be set on option.
 *
 * @return Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function curl_multi_setopt(resource $mh, int $option, mixed $value) : bool;

/**
 * Set multiple options for a cURL transfer
 *
 * @param resource $ch -
 * @param array $options - An array specifying which options to set and
 *   their values. The keys should be valid curl_setopt() constants or
 *   their integer equivalents.
 *
 * @return bool - Returns TRUE if all options were successfully set. If
 *   an option could not be successfully set, FALSE is immediately
 *   returned, ignoring any future options in the options array.
 */
<<__Native>>
function curl_setopt_array(resource $ch,
                           darray $options): bool;

/**
 * Set an option for a cURL transfer
 *
 * @param resource $ch -
 * @param int $option - The CURLOPT_XXX option to set.
 * @param mixed $value - The value to be set on option.
 *
 * @return bool -
 */
<<__Native>>
function curl_setopt(resource $ch,
                     int $option,
                     mixed $value)[write_props]: bool;

/**
 * Gets cURL version information
 *
 * @param int $age -
 *
 * @return array - Returns an associative array with the following
 *   elements:     Indice Value description     version_number cURL 24 bit
 *   version number   version cURL version number, as a string
 *   ssl_version_number OpenSSL 24 bit version number   ssl_version OpenSSL
 *   version number, as a string   libz_version zlib version number, as a
 *   string   host Information about the host where cURL was built   age
 *   features A bitmask of the CURL_VERSION_XXX constants   protocols An
 *   array of protocols names supported by cURL
 */
<<__Native>>
function curl_version(int $age = CURLVERSION_NOW): mixed;

<<__Native>>
function curl_reset(resource $ch): void;

/**
 * Gets options on the given cURL session handle.
 *
 * @param resource $ch - A cURL handle returned by curl_init().
 * @param int $opt     - This should be one of the CURLOPT_* values.
 *
 * @return mixed - If opt is given, returns its value. Otherwise, returns an
 *    associative array.
 */
<<__Native>>
function fb_curl_getopt(resource $ch, int $opt = 0): mixed;

/**
 * extracts file descriptor information from a multi handle.
 *
 * @param resource $mh         - A cURL multi handle returned by
 *     curl_multi_init().
 * @param array& $read_fd_set  - read set
 * @param array& $write_fd_set - write set
 * @param array& $exc_fd_set   - exception set
 * @param int& $max_fd         - If no file descriptors are set, $max_fd will
 *     contain -1. Otherwise it will contain the higher descriptor number.
 *
 * @return mixed - Returns 0 on success, or one of the CURLM_XXX errors code.
 */
<<__Native>>
function fb_curl_multi_fdset(resource $mh,
                              <<__OutOnly("varray")>>
                             inout mixed $read_fd_set,
                              <<__OutOnly("varray")>>
                             inout mixed $write_fd_set,
                              <<__OutOnly("varray")>>
                             inout mixed $exc_fd_set,
                              <<__OutOnly("KindOfInt64")>>
                             inout ?int $max_fd): mixed;

/**
* Returns a new cURL share handle
*
* @return resource - Returns a cURL share handle resource on success,
*/
<<__Native>>
function curl_share_init(): resource;

/**
* Set an option for a cURL share handle.
*
* @param resource $sh -
* @param int $option - One of the CURLSHOPT_* constants.
* @param mixed $value - One of the [CURL_LOCK_DATA_DNS,
*                      CURL_LOCK_DATA_COOKIE, CURL_LOCK_DATA_SSL_SESSION]
*
* @return Returns TRUE on success or FALSE on failure.
*/
<<__Native>>
function curl_share_setopt(resource $sh, int $option, mixed $value) : bool;

/**
 * Close a set of cURL share handles
 *
 * @param resource $sh -
 *
 * @return void -
 */
<<__Native>>
function curl_share_close(resource $sh): void;

} // root namespace

namespace HH\Asio {

/**
 * A convenience wrapper around
 * [`curl_multi_await`](/hack/reference/function/curl_multi_await/).
 *
 * Pass a cURL handle, or, more simply, a string containing a URL (and the
 * cURL handle will be created for you), and the cURL request will be executed
 * via async and the `string` result will be returned.
 *
 * curl_multi_info_read must be used to retrieve error information,
 * curl_errno can't be used as this function is a wrapper to curl_multi_await.
 *
 * @param $urlOrHandle - An existing cURL handle or a URL as a `string`. String
 *                       URLs will create a default cURL GET handle.
 * @param $closeHandleIfHandle - Close cURL handle inside wrapper
 * @return Awaitable<string> - An `Awaitable` representing the `string` result
 *                             of the cURL request.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
async function curl_exec(mixed $urlOrHandle,
                         bool $closeHandleIfHandle = false): Awaitable<string> {
  if (\is_string($urlOrHandle)) {
    $ch = \curl_init($urlOrHandle);
  } else if (\is_resource($urlOrHandle) &&
             (\get_resource_type($urlOrHandle) == "curl")) {
    $ch = $urlOrHandle;
  } else {
    throw new \Exception(__FUNCTION__." expects string of cURL handle");
  }
  \curl_setopt($ch, \CURLOPT_RETURNTRANSFER, true);

  $mh = \curl_multi_init();
  \curl_multi_add_handle($mh, $ch);

  $sleep_count = 0;
  do {
    $active = 1;
    do {
      $status = \curl_multi_exec($mh, inout $active);
    } while ($status == \CURLM_CALL_MULTI_PERFORM);
    if (!$active) break;
    $select = await \curl_multi_await($mh);
    /* If cURL is built without ares support, DNS queries don't have a socket
     * to wait on, so curl_multi_await() (and curl_select() in PHP5) will return
     * -1, and polling is required.
     */
    if ($select == -1) {
      // Slow down polling a bit if it's been a while (>100ms) since we had any work to do.
      $sleep_ms = ($sleep_count >= 10) ? 100 : 10;
      await SleepWaitHandle::create($sleep_ms * 1000);
      $sleep_count += 1;
    } else {
      $sleep_count = 0;
    }
  } while ($status === \CURLM_OK);
  $content = (string)\curl_multi_getcontent($ch);
  \curl_multi_remove_handle($mh, $ch);

  /* close handle if string was passed or argument */
  if (\is_string($urlOrHandle) || ($closeHandleIfHandle === true)) {
    \curl_close($ch);
  }

  \curl_multi_close($mh);
  return $content;
}

} // namespace HH\Asio
