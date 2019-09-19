<?hh // partial

namespace __hhvm_intrinsics {

/**
 * @param bool $oom
 *
 * @return void
 */
<<__Native, __HipHopSpecific>>
function trigger_oom(bool $oom): void;

class ReffyNativeMeth { <<__Native>> static function meth(mixed &$i): string; }

/**
 * Return the given value. This function is purposefully not optimized. It can
 * be used to hide information about values from the optimizer for unit testing.
 */
<<__Native, __HipHopSyntax, __ProvenanceSkipFrame>>
function launder_value(mixed $value): mixed;

/*
 * Builtins for testing array-ish builtin typehints.
 */
<<__Native, __HipHopSyntax>>
function dummy_varray_builtin(varray $x): varray;

<<__Native, __HipHopSyntax>>
function dummy_darray_builtin(darray $x): darray;

<<__Native, __HipHopSyntax>>
function dummy_varr_or_darr_builtin(varray_or_darray $x): varray_or_darray;

<<__Native, __HipHopSyntax>>
function dummy_arraylike_builtin(arraylike $x): arraylike;

<<__Native, __HipHopSyntax>>
function dummy_array_builtin(array $x): array;

<<__Native, __HipHopSyntax>>
function dummy_dict_builtin(dict $x): dict;

<<__Native, __HipHopSyntax>>
function create_class_pointer(string $name): mixed;

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

<<__Native>>
function deserialize_keep_dvarrays(string $str): mixed;

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
