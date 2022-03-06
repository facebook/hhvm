<?hh

function __source(): int { return 1; }
function __sink(int $input): void {}
function __string_source(): string { return "1"; }
function __string_sink(string $input): void {}
function __bool_sink(bool $input): void {}

function source_through_builtin_into_sink(): void {
  $v = __string_source();
  $encoded = lz4_compress($v, false);
  __string_sink($encoded);
}

function source_to_builtin_as_sink(): void {
  $v = __source();
  // Not a flow
  fb_serialize(1, $v);
  // This is a flow
  fb_serialize($v);
}

function builtin_as_source(): void {
  $v = 1;
  $serialized = fb_compact_serialize($v);
  $success = 0;
  $errcode = 0;
  $result = fb_compact_unserialize($serialized, inout $success, inout $errcode);
  __sink($result);
}

// Show we can capture flows through multiple builtins
// Also: if a function is a source, and receives tainted input,
// show that we prefer the longer path, and don't chop it off
// using the function as a source
function builtins_as_tito(): void {
  $v = __source();
  $serialized = fb_compact_serialize($v);
  $success = 0;
  $errcode = 0;
  $result = fb_compact_unserialize($serialized, inout $success, inout $errcode);
  __sink($result);
}

<<__EntryPoint>> function main(): void {
  source_through_builtin_into_sink();
  source_to_builtin_as_sink();
  builtin_as_source();
  builtins_as_tito();
}
