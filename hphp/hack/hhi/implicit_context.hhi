<?hh

namespace HH {

namespace ImplicitContext {




function soft_run_with<Tout>(
  (function ()[_]: Tout) $f,
  string $key,
)[zoned, ctx $f]: Tout;

} // namespace ImplicitContext

abstract class ImplicitContext {
  abstract const type T as nonnull;

  protected static function runWith<Tout>(
    this::T $context,
    (function ()[_]: Tout) $f,
  )[zoned, ctx $f]: Tout;

  protected static function get()[zoned]: ?this::T;
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
