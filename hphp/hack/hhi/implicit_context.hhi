<?hh

namespace HH {

  namespace ImplicitContext {

    async function soft_run_with_async<Tout>(
      (function()[_]: Awaitable<Tout>) $f,
      string $key,
    )[zoned, ctx $f]: Awaitable<Tout>;

    function soft_run_with<Tout>(
      (function()[_]: Tout) $f,
      string $key,
    )[zoned, ctx $f]: Tout;

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

  abstract class ImplicitContext {
    abstract const type T as nonnull;
    abstract const bool IS_MEMO_SENSITIVE;
    abstract const ctx CRun as [leak_safe];

    protected static async function runWithAsync<Tout>(
      this::T $context,
      (function()[_]: Awaitable<Tout>) $f,
    )[this::CRun, ctx $f]: Awaitable<Tout>;

    protected static function runWith<Tout>(
      this::T $context,
      (function()[_]: Tout) $f,
    )[this::CRun, ctx $f]: Tout;

    protected static function get()[this::CRun]: ?this::T;
    protected static function exists()[this::CRun]: bool;
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
