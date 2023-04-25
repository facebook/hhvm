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

    protected static async function runWithAsync<Tout>(
      this::T $context,
      (function()[_]: Awaitable<Tout>) $f,
    )[zoned, ctx $f]: Awaitable<Tout>;

    protected static function runWith<Tout>(
      this::T $context,
      (function()[_]: Tout) $f,
    )[zoned, ctx $f]: Tout;

    protected static function get()[zoned]: ?this::T;
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
     * Calling an "uncategorized" memoized function including one using
     * #SoftMakeICInaccessible will result in an exception.
     */
    string MakeICInaccessible = 'MakeICInaccessible';
    /**
     * Will throw if called with an Implicit Context value.
     * Do not incorporate the Implicit Context state into the memoization cache
     * key.
     * Behaviors that would result in an exception under #MakeICInaccessible
     * will log instead.
     */
    string SoftMakeICInaccessible = 'SoftMakeICInaccessible';
    /**
     * Default option for memoization attributes.
     * Will throw if called with an Implicit Context value.
     * Do not incorporate the Implicit Context state into the memoization cache
     * key.
     */
    string Uncategorized = 'Uncategorized';
  }

  /**
   * States obtainable via get_state_unsafe
   */
  enum State: string as string {
    NULL = 'NULL';
    VALUE = 'VALUE';
    SOFT_SET = 'SOFT_SET';
    INACCESSIBLE = 'INACCESSIBLE';
    SOFT_INACCESSIBLE = 'SOFT_INACCESSIBLE';
  }

} // namespace HH
