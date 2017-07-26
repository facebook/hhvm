<?hh

/**
 * Flushes all response data to the client
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function fastcgi_finish_request(): bool;

