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
  VALUE = 'VALUE';
  INACCESSIBLE = 'INACCESSIBLE';
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

} // namespace ImplicitContext

namespace ImplicitContext\_Private {

<<__NativeData>>
final class ImplicitContextData {}

/**
 * Returns the implicit context keyed by $key or null if such doesn't exist
 */
<<__Native>>
function get_implicit_context<T>(class<T> $key)[leak_safe]: ?T::T;

<<__Native>>
function get_whole_implicit_context()[zoned]: ImplicitContextData;

/**
 * Returns True if the key is present in the IC
 * False otherwise
 *
 * Does not affect the state of the IC
 */
<<__Native>>
function has_key<T>(class<T> $key)[leak_safe]: bool;

/**
 * Creates implicit context $context keyed by $key.
 */
<<__Native>>
function create_implicit_context<T>(
  class<T> $key,
  mixed $context,
  bool $memo_sensitive,
)[leak_safe]: ImplicitContextData;

/**
 * Creates memo agnostic implicit context $context keyed by $key.
 */
<<__Native>>
function create_memo_agnostic<T as \HH\MemoAgnosticImplicitContext>(
  class<T> $key,
  mixed $context,
)[leak_safe]: ImplicitContextData;

/**
 * Creates memo sensitive implicit context $context keyed by $key,
 * with $context's instance key provided by $context_key.
 */
<<__Native>>
function create_memo_sensitive<T as \HH\MemoSensitiveImplicitContext>(
  class<T> $key,
  \HH\IPureMemoizeParam $context,
  string $context_key,
)[leak_safe]: ImplicitContextData;

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

abstract class MemoAgnosticImplicitContext extends ImplicitContextBase {
  final protected static async function runWithAsync<Tout>(
    this::TData $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[this::CRun, ctx $f]: Awaitable<Tout> {
    return await self::runWithImplAsync(self::createContext($context), $f);
  }

  final protected static function runWith<Tout>(
    this::TData $context,
    (function ()[_]: Tout) $f,
  )[this::CRun, ctx $f]: Tout {
    return self::runWithImpl(self::createContext($context), $f);
  }

  private static function createContext(
    this::TData $context,
  ): ImplicitContext\_Private\ImplicitContextData {
    return ImplicitContext\_Private\create_memo_agnostic(
      static::class,
      $context,
    );
  }
}

abstract class MemoSensitiveImplicitContext extends ImplicitContextBase {
  abstract const type TData as IPureMemoizeParam;

  final protected static async function runWithAsync<Tout>(
    this::TData $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[this::CRun, ctx $f]: Awaitable<Tout> {
    return await self::runWithImplAsync(self::createContext($context), $f);
  }

  final protected static function runWith<Tout>(
    this::TData $context,
    (function ()[_]: Tout) $f,
  )[this::CRun, ctx $f]: Tout {
    return self::runWithImpl(self::createContext($context), $f);
  }

  private static function createContext(
    this::TData $context,
  ): ImplicitContext\_Private\ImplicitContextData {
    return ImplicitContext\_Private\create_memo_sensitive(
      static::class,
      $context,
      $context->getInstanceKey(),
    );
  }
}

<<__Sealed(MemoAgnosticImplicitContext::class, MemoSensitiveImplicitContext::class)>>
abstract class ImplicitContextBase {
  abstract const type TData as nonnull;
  abstract const ctx CRun as [leak_safe];

  protected static function exists()[this::CRun]: bool {
    return ImplicitContext\_Private\has_key(static::class);
  }

  protected static function get()[this::CRun]: ?this::TData {
    return ImplicitContext\_Private\get_implicit_context(static::class);
  }

  final protected static async function runWithImplAsync<Tout>(
    ImplicitContext\_Private\ImplicitContextData $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[this::CRun, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value($context);
    try {
      $result = $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
    // Needs to be awaited here so that context dependency is established
    // between parent/child functions
    return await $result;
  }

  final protected static function runWithImpl<Tout>(
    ImplicitContext\_Private\ImplicitContextData $context,
    (function ()[_]: Tout) $f,
  )[this::CRun, ctx $f]: Tout {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value($context);
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }
}

abstract class ImplicitContext {
  abstract const type T as nonnull;
  abstract const bool IS_MEMO_SENSITIVE;
  abstract const ctx CRun as [leak_safe];

  protected static async function runWithAsync<Tout>(
    this::T $context,
    (function ()[_]: Awaitable<Tout>) $f,
  )[this::CRun, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      ImplicitContext\_Private\create_implicit_context(
        static::class,
        $context,
        static::IS_MEMO_SENSITIVE,
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
  )[this::CRun, ctx $f]: Tout {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      ImplicitContext\_Private\create_implicit_context(
        static::class,
        $context,
        static::IS_MEMO_SENSITIVE,
      ),
    );
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  protected static function exists()[this::CRun]: bool {
    return ImplicitContext\_Private\has_key(static::class);
  }

  protected static function get()[this::CRun]: ?this::T {
    return ImplicitContext\_Private\get_implicit_context(static::class);
  }
}

/**
 * Options for memoization to be used with dynamically enforced implicit context
 */
enum class MemoizeOption: string {
  string KeyedByIC = 'KeyedByIC';
  string MakeICInaccessible = 'MakeICInaccessible';
  string NotKeyedByICAndLeakIC = 'NotKeyedByICAndLeakIC__DO_NOT_USE';
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
      \HH\ImplicitContext\_Private\get_memo_agnostic_implicit_context(),
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
      \HH\ImplicitContext\_Private\get_memo_agnostic_implicit_context()
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

} // namespace HH\Coeffects
