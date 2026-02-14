<?hh

namespace HH {

  namespace ImplicitContext {

    /**
     * Captures the current IC state and returns a new closure that calls the
     * input closure using that state.
     *
     * Use this when deferring or batching closures under a different IC state
     * than when they may be executed.
     */
    function embed_implicit_context_state_in_closure<T>(
      (function ()[defaults]: T) $f,
    )[zoned]: (function ()[defaults]: T);

    /**
     * Variation of embed_implicit_context_state_in_closure
     * specifically for async closures
     */
    function embed_implicit_context_state_in_async_closure<T>(
      (function ()[defaults]: Awaitable<T>) $f,
    )[zoned]: (function ()[defaults]: Awaitable<T>);

  } // namespace ImplicitContext

  // Avoid referencing an internal class in OSS.
  // @oss-disable: <<__Sealed(FBMemoAgnosticImplicitContext::class)>>
  abstract class MemoAgnosticImplicitContext extends ImplicitContextBase {
    final protected static async function runWithAsync<Tout>(
      this::TData $context,
      (function ()[_]: Awaitable<Tout>) $f,
    )[leak_safe, ctx $f]: Awaitable<Tout>;

    final protected static function runWith<Tout>(
      this::TData $context,
      (function ()[_]: Tout) $f,
    )[leak_safe, ctx $f]: Tout;
  }

  // Avoid referencing an internal class in OSS.
  // @oss-disable: <<__Sealed(FBMemoSensitiveImplicitContext::class)>>
  abstract class MemoSensitiveImplicitContext extends ImplicitContextBase {
    abstract const type TData as IPureMemoizeParam;

    final protected static async function runWithAsync<Tout>(
      this::TData $context,
      (function ()[_]: Awaitable<Tout>) $f,
    )[leak_safe, ctx $f]: Awaitable<Tout>;

    final protected static function runWith<Tout>(
      this::TData $context,
      (function ()[_]: Tout) $f,
    )[leak_safe, ctx $f]: Tout;
  }

  <<__Sealed(MemoAgnosticImplicitContext::class, MemoSensitiveImplicitContext::class)>>
  abstract class ImplicitContextBase {
    abstract const type TData as nonnull;
    abstract const ctx CRun as [leak_safe];

    protected static function get()[this::CRun]: ?this::TData;

    abstract protected static function runWithAsync<Tout>(
      this::TData $context,
      (function ()[_]: Awaitable<Tout>) $f,
    )[leak_safe, ctx $f]: Awaitable<Tout>;

    abstract protected static function runWith<Tout>(
      this::TData $context,
      (function ()[_]: Tout) $f,
    )[leak_safe, ctx $f]: Tout;
  }

  /**
   * Options for memoization to be used with dynamically enforced implicit
   * context
   */
  enum class MemoizeOption: string {
    /**
     * Incorporate the Implicit Context state into the memoization cache key.
     */
    string KeyedByIC = 'KeyedByIC';
    /**
     * Do not incorporate the Implicit Context state into the memoization cache
     * key.
     * Attempting to fetch the Implicit Context in this function or a recursive
     * callee will result in an exception.
     */
    string MakeICInaccessible = 'MakeICInaccessible';
    /**
     * Do not incorporate the Implicit Context state into the memoization cache
     * key.
     * Attempting to fetch the Implicit Context is allowed.
     * Do not use this unless you know what you're doing.
     */
    string NotKeyedByICAndLeakIC__DO_NOT_USE = 'NotKeyedByICAndLeakIC__DO_NOT_USE';

  }

  /**
   * States obtainable via get_state_unsafe
   */
  enum State: string as string {
    VALUE = 'VALUE';
    INACCESSIBLE = 'INACCESSIBLE';
  }

} // namespace HH
