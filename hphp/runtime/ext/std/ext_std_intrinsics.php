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
<<__Native, __HipHopSyntax>>
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
function create_class_pointer(string $name): mixed;

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
