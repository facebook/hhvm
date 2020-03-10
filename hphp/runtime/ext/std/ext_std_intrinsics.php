<?hh // partial

namespace __hhvm_intrinsics {

/**
 * @param bool $oom
 *
 * @return void
 */
<<__Native, __HipHopSpecific>>
function trigger_oom(bool $oom): void;

/**
 * Return the given value. This function is purposefully not optimized. It can
 * be used to hide information about values from the optimizer for unit testing.
 */
<<__Native, __HipHopSpecific, __ProvenanceSkipFrame>>
function launder_value(mixed $value): mixed;

/*
 * Builtins for testing array-ish builtin typehints.
 */
<<__Native, __HipHopSpecific>>
function dummy_varray_builtin(varray $x): varray;

<<__Native, __HipHopSpecific>>
function dummy_darray_builtin(darray $x): darray;

<<__Native, __HipHopSpecific>>
function dummy_kindofdarray_builtin(): mixed;

<<__Native, __HipHopSpecific>>
function dummy_kindofvarray_builtin(): mixed;

<<__Native, __HipHopSpecific>>
function dummy_varr_or_darr_builtin(varray_or_darray $x): varray_or_darray;

<<__Native, __HipHopSpecific>>
function dummy_arraylike_builtin(arraylike $x): arraylike;

<<__Native, __HipHopSpecific>>
function dummy_cast_to_kindofarray(arraylike $value): mixed;

<<__Native, __HipHopSpecific>>
function dummy_cast_to_kindofdarray(array $value): mixed;

<<__Native, __HipHopSpecific>>
function dummy_cast_to_kindofvarray(array $value): mixed;

<<__Native, __HipHopSpecific>>
function dummy_array_builtin(array $x): array;

<<__Native, __HipHopSpecific>>
function dummy_dict_builtin(dict $x): dict;

<<__Native, __HipHopSpecific>>
function dummy_array_await(): Awaitable;

<<__Native, __HipHopSpecific>>
function dummy_darray_await(): Awaitable;

<<__Native, __HipHopSpecific>>
function dummy_dict_await(): Awaitable;

<<__Native, __HipHopSpecific>>
function create_class_pointer(string $name): mixed;

<<__Native, __HipHopSpecific>>
function create_clsmeth_pointer(string $cls, string $meth): mixed;

function apc_fetch_no_check(mixed $key) {
  $ignored = false;
  return \apc_fetch($key, inout $ignored);
}

<<__Native, __IsFoldable>>
function builtin_io_foldable(
  int $a,
  inout int $b,
  inout int $c,
  inout int $d,
): int;

<<__Native>>
function builtin_io(
  string $s,
  inout string $str,
  inout int $num,
  int $i,
  inout object $obj,
  object $o,
  mixed $m,
  inout mixed $mix,
  bool $retOrig,
  <<__OutOnly("KindOfBoolean")>> inout mixed $out1,
  <<__OutOnly("KindOfArray")>> inout mixed $out2,
  <<__OutOnly("KindOfObject")>> inout mixed $out3,
): array;

<<__Native("NoFCallBuiltin")>>
function builtin_io_no_fca(
  string $s,
  inout string $str,
  inout int $num,
  int $i,
  inout object $obj,
  object $o,
  mixed $m,
  inout mixed $mix,
  bool $retOrig,
  <<__OutOnly("KindOfBoolean")>> inout mixed $out1,
  <<__OutOnly("KindOfArray")>> inout mixed $out2,
  <<__OutOnly("KindOfObject")>> inout mixed $out3,
): array;

/*
 * Like serialize(), but serialize d/varrays into their own format so that they
 * can be distinguished and deserialized as themselves (serialize() will
 * serialize them as normal PHP arrays).
 */
<<__IsFoldable, __Native>>
function serialize_keep_dvarrays(mixed $value): string;

/* dummy builtin written in hack for testing param coercion */
function id_string(string $value): string {
  return $value;
}

<<__Native>>
function serialize_with_format(mixed $thing, int $format): string;

<<__Native>>
function rqtrace_create_event(
  string $name,
  int $start_us,
  int $end_us,
  dict<string, string> $annot
): void;

<<__Native>>
function rqtrace_create_scope(
  string $name,
  int $start_us,
  int $end_us,
  dict<string, string> $annot
): void;

<<__Native>>
function rqtrace_create_scoped_events(
  string $name,
  int $start_us,
  int $end_us,
  string $prefix,
  string $suffix,
  dict<string, string> $annot,
  dict<string, (int, int, dict<string, string>)> $events
): void;

/* dummy builtin to cause hhbbc to emit unverifiable bytecode */
<<__Native>>
function hhbbc_fail_verification(): void;

}
