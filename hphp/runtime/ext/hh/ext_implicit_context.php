<?hh

namespace HH {

namespace ImplicitContext {

async function soft_run_with_async<Tout>(
  (function ()[_]: Awaitable<Tout>) $f,
  string $key,
)[zoned, ctx $f]: Awaitable<Tout> {
  return await $f();
}

function soft_run_with<Tout>(
  (function ()[_]: Tout) $f,
  string $key,
)[zoned, ctx $f]: Tout {
  return $f();
}

const string RUN_WITH_SOFT_INACCESSIBLE_KEY_PREFIX = 'nonfunction/';


function embed_implicit_context_state_in_closure<T>(
  (function ()[defaults]: T) $f,
)[zoned]: (function ()[defaults]: T) {
  $captured_ic_state = _Private\get_whole_implicit_context();
  return ()[defaults] ==> {
    $prev = _Private\set_implicit_context_by_value($captured_ic_state);
    try {
      return $f();
    } finally {
      _Private\set_implicit_context_by_value($prev);
    }
  };
}

function embed_implicit_context_state_in_async_closure<T>(
  (function ()[defaults]: Awaitable<T>) $f,
)[zoned]: (function ()[defaults]: Awaitable<T>) {
  $captured_ic_state = _Private\get_whole_implicit_context();
  return async ()[defaults] ==> {
    $prev = _Private\set_implicit_context_by_value($captured_ic_state);
    try {
      $awaitable = $f();
    } finally {
      _Private\set_implicit_context_by_value($prev);
    }
    // Needs to be awaited here so that context dependency is established
    // between parent/child functions
    return await $awaitable;
  };
}

enum State: string as string {
  NULL = 'NULL';
  VALUE = 'VALUE';
  SOFT_SET = 'SOFT_SET';
  INACCESSIBLE = 'INACCESSIBLE';
  SOFT_INACCESSIBLE = 'SOFT_INACCESSIBLE';
}

/**
 * This function returns a value of the `HH\ImplicitContext\State` enum
 * corresponding to the current IC state.
 *
 * This function is unsafe in that it doesn't trigger exceptions/logs for
 * observing IC state when you normally should not. Use with caution.
 */
<<__Native>>
function get_state_unsafe()[zoned]: string /* State */;

/**
 * Returns True if we are in the empty IC state
 * False otherwise
 *
 * Does not affect the state of the IC
 */
<<__Native>>
function is_inaccessible()[zoned]: bool;

} // namespace ImplicitContext

namespace ImplicitContext\_Private {

<<__NativeData>>
final class ImplicitContextData {}

/**
 * Returns the implicit context keyed by $key or null if such doesn't exist
 */
<<__Native>>
function get_implicit_context(string $key)[zoned]: mixed;

<<__Native>>
function get_whole_implicit_context()[zoned]: ImplicitContextData;

/**
 * Returns True if the key is present in the IC
 * False otherwise
 *
 * Does not affect the state of the IC
 */
<<__Native>>
function has_key(string $key)[zoned]: bool;

/**
 * Creates implicit context $context keyed by $key.
 */
<<__Native>>
function create_implicit_context(
  string $key,
  mixed $context,
  bool $memo_sensitive,
)[zoned]: ImplicitContextData;

/*
 * Returns the currently implicit context hash or empty string if
 * no implicit context is set
 */
<<__Native("NoRecording")>>
function get_implicit_context_memo_key()[zoned]: int;

/*
* Returns the constituents of the current implicit context key
* Only to be used for tests
*/
<<__Native("NoRecording")>>
function get_implicit_context_debug_info()[]: vec<string>;


} // namespace ImplicitContext_Private

abstract class ImplicitContext {
  abstract const type T as nonnull;

  protected static async function runWithAsync<Tout>(
    this::T $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[zoned, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      ImplicitContext\_Private\create_implicit_context(
        nameof static,
        $context,
        true,
      ),
    );
    try {
      $result = $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
    // Needs to be awaited here so that context dependency is established
    // between parent/child functions
    return await $result;
  }

  protected static function runWith<Tout>(
    this::T $context,
    (function ()[_]: Tout) $f,
  )[zoned, ctx $f]: Tout {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      ImplicitContext\_Private\create_implicit_context(
        nameof static,
        $context,
        true,
      ),
    );
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  protected static function exists()[zoned]: bool {
    return ImplicitContext\_Private\has_key(nameof static);
  }

  protected static function get()[zoned]: ?this::T {
    return ImplicitContext\_Private\get_implicit_context(nameof static);
  }
}

/**
 * Options for memoization to be used with dynamically enforced implicit context
 */
enum class MemoizeOption: string {
  string KeyedByIC = 'KeyedByIC';
  string MakeICInaccessible = 'MakeICInaccessible';
  string SoftMakeICInaccessible = 'SoftMakeICInaccessible';
  string Uncategorized = 'Uncategorized';
}

} // namespace HH

namespace HH\Coeffects {

  /**
   * Creates an unsafe way to call a function by providing defaults coeffects.
   * EXTREMELY UNSAFE. USE WITH CAUTION.
   */
  function backdoor<Tout>(
    (function()[defaults]: Tout) $fn
  )[/* 86backdoor */]: Tout {
    $prev = \HH\ImplicitContext\_Private\set_implicit_context_by_value(
      \HH\ImplicitContext\_Private\get_inaccessible_implicit_context(),
    );
    try {
      return $fn();
    } finally {
      \HH\ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  /**
   * Creates an unsafe way to call a function by providing defaults coeffects.
   * EXTREMELY UNSAFE. USE WITH CAUTION.
   */
  async function backdoor_async<Tout>(
    (function()[defaults]: Awaitable<Tout>) $fn
  )[/* 86backdoor */]: Awaitable<Tout> {
    $prev = \HH\ImplicitContext\_Private\set_implicit_context_by_value(
      \HH\ImplicitContext\_Private\get_inaccessible_implicit_context()
    );
    try {
      $result = $fn();
    } finally {
      \HH\ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
    // Needs to be awaited here so that context dependency is established
    // between parent/child functions
    return await $result;
  }

  namespace _Private {

  /**
   * The internal entry point for zoned_with functions
   */
  <<__Native>>
  function enter_zoned_with<Tout, Tpolicy>(
    (function()[zoned_with<Tpolicy>]: Tout) $f
  )[zoned]: mixed /* Tout */;

  } // namespace _Private

  /**
   * The public entry point for zoned_with functions
   */
  function enter_zoned_with<Tout, Tcontext as ImplicitContext, Tval>(
    classname<Tcontext> $cls,
    Tval $value,
    (function()[zoned_with<Tval>]: Tout) $f,
  )[zoned]: Tout where Tval = Tcontext::T {
    return $cls::set(
      $value,
      ()[zoned] ==> _Private\enter_zoned_with($f)
    );
  }

  /**
   * The public entry point for async zoned_with functions
   */
  async function enter_zoned_with_async<
    Tout,
    Tcontext as ImplicitContext,
    Tval
  >(
    classname<Tcontext> $cls,
    Tval $value,
    (function()[zoned_with<Tval>]: Awaitable<Tout>) $f,
  )[zoned]: Awaitable<Tout> where Tval = Tcontext::T {
    return await $cls::setAsync(
      $value,
      ()[zoned] ==> _Private\enter_zoned_with($f)
    );
  }

} // namespace HH\Coeffects
