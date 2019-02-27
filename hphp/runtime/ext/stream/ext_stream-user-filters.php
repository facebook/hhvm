<?hh

<<__Native>>
function stream_get_filters(): array;

<<__Native>>
function stream_filter_register(string $filtername, string $classname): bool;

<<__Native>>
function stream_filter_append(
  resource $stream,
  string $filtername,
  ?int $readwrite = null,
  ?mixed $params = null
): mixed;

<<__Native>>
function stream_filter_prepend(
  resource $stream,
  string $filtername,
  ?int $readwrite = null,
  ?mixed $params = null
): mixed;

<<__Native>>
function stream_filter_remove(
  resource $stream_filter
): bool;

/* Zend documentation inconsistency:
 * - stream_bucket_make_writeable returns a bucket object
 * - you pass this to stream_bucket_append or similar
 * - these document bucket as being a resource, not an object
 * - it's actually an STDclass
 *
 * We use a __SystemLib\StreamFilterBucket object throughout.
 */

<<__Native>>
function stream_bucket_make_writeable(resource $brigade): ?object;

<<__Native>>
function stream_bucket_append(resource $brigade, object $bucket): void;

<<__Native>>
function stream_bucket_prepend(resource $brigade, object $bucket): void;
