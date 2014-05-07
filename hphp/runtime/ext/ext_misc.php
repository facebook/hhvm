<?hh

namespace HH {

/**
 * Returns a description of the warmup status of the server.
 *
 * @return string - If the server appears to be warmed up, returns the empty
 * string. Otherwise, returns a human-readable description of why the server is
 * not warmed up. Note that this function checks a series of heuristics rather
 * than anything definitive; returning '' for one request does not guarantee
 * the same result for subsequent requests.
 */
<<__Native>>
function server_warmup_status(): string;

}
