<?hh // partial

namespace HH {

namespace ImplicitContext {

async function soft_run_with_async<Tout>(
  (function ()[_]: Awaitable<Tout>) $f,
  string $key,
)[zoned, ctx $f]: Awaitable<Tout> {
  $prev = _Private\set_implicit_context_by_value(
    _Private\create_special_implicit_context(
      \HH\MEMOIZE_IC_TYPE_SOFT_SET,
      $key
    ),
  );
  try {
    $result = $f();
  } finally {
    _Private\set_implicit_context_by_value($prev);
  }
  // Needs to be awaited here so that context dependency is established
  // between parent/child functions
  return await $result;
}

function soft_run_with<Tout>(
  (function ()[_]: Tout) $f,
  string $key,
)[zoned, ctx $f]: Tout {
  $prev = _Private\set_implicit_context_by_value(
    _Private\create_special_implicit_context(
      \HH\MEMOIZE_IC_TYPE_SOFT_SET,
      $key
    ),
  );
  try {
    return $f();
  } finally {
    _Private\set_implicit_context_by_value($prev);
  }
}

function embed_implicit_context_state_in_closure(
  (function ()[defaults]: void) $f,
)[zoned]: (function ()[defaults]: void) {
  $captured_ic_state = _Private\get_whole_implicit_context();
  return ()[defaults] ==> {
    $prev = _Private\set_implicit_context_by_value($captured_ic_state);
    try {
      $f();
    } finally {
      _Private\set_implicit_context_by_value($prev);
    }
  };
}

function embed_implicit_context_state_in_async_closure(
  (function ()[defaults]: Awaitable<void>) $f,
)[zoned]: (function ()[defaults]: Awaitable<void>) {
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
    await $awaitable;
  };
}

} // namespace ImplicitContext

namespace ImplicitContext\_Private {

<<__NativeData("ImplicitContext")>>
final class ImplicitContextData {}

/**
 * Returns the implicit context keyed by $key or null if such doesn't exist
 */
<<__Native>>
function get_implicit_context(string $key)[zoned]: mixed;

<<__Native>>
function get_whole_implicit_context()[zoned]: object /* ImplicitContextData */;

/**
 * Sets implicit context $context keyed by $key.
 * Returns the previous implicit context.
 */
<<__Native>>
function set_implicit_context(
  string $key,
  mixed $context,
)[zoned]: object /* ImplicitContextData */;

/**
 * Sets implicit context to one of the non value based special types.
 * The type value is of SpecialImplicitContextType.
 * If $memo_key is provided, it is used for keying the memoization key,
 * otherwise name of the caller is used.
 * Returns the previous implicit context.
 *
 * NOTE: This code is actually [zoned] but it is safe to call from
 * [leak_safe_shallow] since leak_safe_shallow can call it via a level of
 * indirection. However, this happens in HackC generated memoized wrapped code.
 * Mark this code as [leak_safe] to avoid this level of indirection.
 * This code should not be called from userland.
 */
<<__Native>>
function create_special_implicit_context(
  int $type /* SpecialImplicitContextType */,
  ?string $memo_key = null,
)[leak_safe]: object /* ImplicitContextData */;

/*
 * Singleton memoization wrapper over create_special_implicit_context for
 * ic inaccessible case
 */
<<__Memoize>>
function create_ic_inaccessible_context()[] {
  // Note: This function needs a backdoor since it needs to call zoned code
  // but it does not actually inspect the IC
  // The parent function cannot be zoned since zoned requires a memoization
  // category which will result in infinite loop since MakeICInaccessible
  // uses this function.
  return \HH\Coeffects\backdoor(
    ()[zoned] ==> create_special_implicit_context(\HH\MEMOIZE_IC_TYPE_INACCESSIBLE),
  );
}

/*
 * Returns the currently implicit context hash or empty string if
 * no implicit context is set
 */
<<__Native>>
function get_implicit_context_memo_key()[zoned]: string;

} // namespace ImplicitContext_Private

abstract class ImplicitContext {
  abstract const type T as nonnull;

  protected static async function runWithAsync<Tout>(
    this::T $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[zoned, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context(
      static::class,
      $context,
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
    $prev = ImplicitContext\_Private\set_implicit_context(
      static::class,
      $context,
    );
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  protected static function get()[zoned]: ?this::T {
    return ImplicitContext\_Private\get_implicit_context(static::class);
  }
}

/**
 * Options for memoization to be used with dynamically enforced implicit context
 */
enum class MemoizeOption: string {
  string KeyedByIC = 'KeyedByIC';
  string MakeICInaccessible = 'MakeICInaccessible';
  string SoftMakeICInaccessible = 'SoftMakeICInaccessible';
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
    $prev = \HH\ImplicitContext\_Private\set_implicit_context_by_value(null);
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
    $prev = \HH\ImplicitContext\_Private\set_implicit_context_by_value(null);
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

