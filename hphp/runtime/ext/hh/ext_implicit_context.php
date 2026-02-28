<?hh

namespace HH {

namespace ImplicitContext {

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

<<__Sealed(
  \HH\ImplicitContext\_Private\MemoAgnosticPreparedContext::class,
  \HH\ImplicitContext\_Private\MemoSensitivePreparedContext::class,
)>>
abstract class PreparedContext {
  final public function __construct(
    private class<ImplicitContextBase> $icClass,
    private mixed $context,
  )[] {}

  /**
   * Execute a closure with all prepared contexts active.
   * Contexts are nested in the order they appear (first = outermost).
   */
  final public static async function runBatchAsync<Tout>(
    vec<\HH\ImplicitContext\PreparedContext> $prepared,
    (function ()[_]: Awaitable<Tout>) $f,
  )[leak_safe, ctx $f]: Awaitable<Tout> {
    invariant(!\HH\Lib\C\is_empty($prepared), 'Must have at least one context');

    // TODO: fold these context creations into a single one.
    try {
      $first_context = null;
      foreach ($prepared as $prepared_context) {
        $ic_class = $prepared_context->icClass;
        $context = $prepared_context->context;
        if ($prepared_context is \HH\ImplicitContext\_Private\MemoAgnosticPreparedContext) {
          $next_context = \HH\ImplicitContext\_Private\create_memo_agnostic(
            $ic_class,
            $context,
          );
        } else if ($prepared_context is \HH\ImplicitContext\_Private\MemoSensitivePreparedContext) {
          $next_context = \HH\ImplicitContext\_Private\create_memo_sensitive(
            $ic_class,
            $context,
            $context->getInstanceKey()
          );
        } else {
          invariant_violation(
            'Unexpected prepared context type: %s',
            \get_class($prepared_context),
          );
        }
        $prev_context = \HH\ImplicitContext\_Private\set_implicit_context_by_value(
          $next_context
        );
        $first_context ??= $prev_context;
      }

      $result = $f();
    } finally {
      if ($first_context is nonnull) {
        \HH\ImplicitContext\_Private\set_implicit_context_by_value(
          $first_context,
        );
      }
    }
    // Needs to be awaited here so that context dependency is established
    // between parent/child functions
    return await $result;
  }

  /**
   * Synchronous variant.
   */
  final public static function runBatch<Tout>(
    vec<\HH\ImplicitContext\PreparedContext> $prepared,
    (function ()[_]: Tout) $f,
  )[leak_safe, ctx $f]: Tout {
    invariant(!\HH\Lib\C\is_empty($prepared), 'Must have at least one context');

    // TODO: fold these context creations into a single one.
    try {
      $first_context = null;
      foreach ($prepared as $prepared_context) {
        $ic_class = $prepared_context->icClass;
        $context = $prepared_context->context;
        if ($prepared_context is \HH\ImplicitContext\_Private\MemoAgnosticPreparedContext) {
          $next_context = \HH\ImplicitContext\_Private\create_memo_agnostic(
            $ic_class,
            $context,
          );
        } else if ($prepared_context is \HH\ImplicitContext\_Private\MemoSensitivePreparedContext) {
          $next_context = \HH\ImplicitContext\_Private\create_memo_sensitive(
            $ic_class,
            $context,
            $context->getInstanceKey()
          );
        } else {
          invariant_violation(
            'Unexpected prepared context type: %s',
            \get_class($prepared_context),
          );
        }
        $prev_context = \HH\ImplicitContext\_Private\set_implicit_context_by_value(
          $next_context
        );
        $first_context ??= $prev_context;
      }

      return $f();
    } finally {
      if ($first_context is nonnull) {
        \HH\ImplicitContext\_Private\set_implicit_context_by_value(
          $first_context,
        );
      }
    }
  }
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

final class MemoAgnosticPreparedContext extends \HH\ImplicitContext\PreparedContext {
}

final class MemoSensitivePreparedContext extends \HH\ImplicitContext\PreparedContext {
}

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
  )[leak_safe, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      self::createContext($context)
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

  final protected static function runWith<Tout>(
    this::TData $context,
    (function ()[_]: Tout) $f,
  )[leak_safe, ctx $f]: Tout {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      self::createContext($context)
    );
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  final protected static function prepare(
    this::TData $context,
  )[]: \HH\ImplicitContext\PreparedContext {
    return new \HH\ImplicitContext\_Private\MemoAgnosticPreparedContext(
      static::class,
      $context,
    );
  }

  private static function createContext(
    this::TData $context,
  )[leak_safe]: ImplicitContext\_Private\ImplicitContextData {
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
  )[leak_safe, ctx $f]: Awaitable<Tout> {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      self::createContext($context)
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

  final protected static function runWith<Tout>(
    this::TData $context,
    (function ()[_]: Tout) $f,
  )[leak_safe, ctx $f]: Tout {
    $prev = ImplicitContext\_Private\set_implicit_context_by_value(
      self::createContext($context)
    );
    try {
      return $f();
    } finally {
      ImplicitContext\_Private\set_implicit_context_by_value($prev);
    }
  }

  final protected static function prepare(
    this::TData $context,
  )[]: \HH\ImplicitContext\PreparedContext {
    return new \HH\ImplicitContext\_Private\MemoSensitivePreparedContext(
      static::class,
      $context,
    );
  }

  private static function createContext(
    this::TData $context,
  )[leak_safe]: ImplicitContext\_Private\ImplicitContextData {
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

  protected static function get()[this::CRun]: ?this::TData {
    return ImplicitContext\_Private\get_implicit_context(static::class);
  }

  abstract protected static function runWithAsync<Tout>(
    this::TData $context,
    (function()[_]: Awaitable<Tout>) $f,
  )[leak_safe, ctx $f]: Awaitable<Tout>;

  abstract protected static function runWith<Tout>(
    this::TData $context,
    (function()[_]: Tout) $f,
  )[leak_safe, ctx $f]: Tout;

  abstract protected static function prepare(
    this::TData $context,
  )[]: \HH\ImplicitContext\PreparedContext;
}

// These will come handy if we decide to seal the PHP classes
// All HHVM tests now inherit only from these 2 classes
abstract class HHVMTestMemoSensitiveImplicitContext extends MemoSensitiveImplicitContext {}
abstract class HHVMTestMemoAgnosticImplicitContext extends MemoAgnosticImplicitContext {}


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
