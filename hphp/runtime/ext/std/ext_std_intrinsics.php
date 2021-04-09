<?hh // partial

namespace __hhvm_intrinsics {

/**
 * @param bool $oom
 *
 * @return void
 */
<<__Native>>
function trigger_oom(bool $oom): void;

/**
 * Return the given value. This function is purposefully not optimized. It can
 * be used to hide information about values from the optimizer for unit testing.
 */
<<__Native, __ProvenanceSkipFrame>>
function launder_value(mixed $value): mixed;

/*
 * Builtins for testing array-ish builtin typehints.
 */
<<__Native>>
function dummy_varray_builtin(varray $x): varray;

<<__Native>>
function dummy_darray_builtin(darray $x): darray;

<<__Native>>
function dummy_kindofdarray_builtin(): mixed;

<<__Native>>
function dummy_kindofvarray_builtin(): mixed;

<<__Native>>
function dummy_varr_or_darr_builtin(varray_or_darray $x): varray_or_darray;

<<__Native>>
function dummy_arraylike_builtin(AnyArray $x): AnyArray;

<<__Native>>
function dummy_dict_builtin(dict $x): dict;

<<__Native>>
function dummy_array_await(): Awaitable;

<<__Native>>
function dummy_darray_await(): Awaitable;

<<__Native>>
function dummy_dict_await(): Awaitable;

<<__Native>>
function create_class_pointer(string $name): mixed;

<<__Native>>
function create_clsmeth_pointer(string $cls, string $meth): mixed;

<<__Native>>
function dummy_lots_inout(inout $p1, inout $p2, inout $p3, inout $p4,
                          inout $p1, inout $p2, inout $p3, inout $p4,
                          inout $p1, inout $p2, inout $p3, inout $p4): mixed;

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
  <<__OutOnly("varray")>> inout mixed $out2,
  <<__OutOnly("KindOfObject")>> inout mixed $out3,
): varray;

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
  <<__OutOnly("varray")>> inout mixed $out2,
  <<__OutOnly("KindOfObject")>> inout mixed $out3,
): varray;

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
function deserialize_with_format(string $str, int $format): mixed;

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

/* Test if the unit given by the path has been loaded. Only valid in
 * non-RepoAuth mode */
<<__Native>>
function is_unit_loaded(string $path): bool;

/*
 * Block until there's no outstanding unit prefetches. This can block
 * indefinitely if we keep trying to prefetch units, so use with
 * care.
 */
<<__Native>>
function drain_unit_prefetcher(): void;

/*
 * Return information about the current contents of the non-repo Unit
 * cache and per-hash Unit cache. Due to the nature of the caches this
 * information may be out of date immediately. Only really meant for
 * Unit tests.
 */
<<__Native>>
function non_repo_unit_cache_info(): dict;

}
