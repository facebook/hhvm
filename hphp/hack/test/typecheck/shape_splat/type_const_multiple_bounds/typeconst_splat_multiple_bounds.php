<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'type_const_multiple_bounds',
)>>

// Splatting a type constant declared with several `as` bounds.
//
// A type constant's bounds are intersected before the splat ever sees them,
// unlike a type parameter's, which stay separate. Where that intersection is
// uninhabited the element is a subtype of `nothing`, so normalization takes the
// bottom-row arm and the whole shape becomes `nothing`. Well-formedness stays
// quiet for the same reason: `nothing` is the bottom row, a legitimate splat
// element.
//
// That is sound - no concrete subclass can define such a type constant, so the
// method is unreachable, but it is silent.

interface I {}
interface J {}

abstract class A {
  // Both bounds spreadable: they fold into a single shape.
  abstract const type TTwoShapes as
    shape('a' => int, ...)
    as shape('b' => bool, ...);

  // Spreadable plus a jointly satisfiable non-shape bound.
  abstract const type TShapeNonnull as shape('a' => int, ...) as nonnull;

  // Jointly satisfiable, neither bound spreadable.
  abstract const type TIfaces as I as J;

  // Uninhabited: `shape('a' => int, ...) & I` is `nothing`.
  abstract const type TShapeAndIface as shape('a' => int, ...) as I;

  // Uninhabited: `I & arraykey` is `nothing`.
  abstract const type TNeither as I as arraykey;

  public function two_shapes(shape(...this::TTwoShapes) $s): void {
    hh_expect<int>($s['a']);
    hh_expect<bool>($s['b']);
  }

  // The non-shape bound does not stop the shape bound being read.
  public function shape_nonnull(shape(...this::TShapeNonnull) $s): void {
    hh_expect_equivalent<this::TShapeNonnull>($s);
    hh_expect<int>($s['a']);
  }

  // REJECT: no bound can be spread.
  public function ifaces(shape(...this::TIfaces) $s): void {}

  // Accepted, and the whole row is bottom rather than `shape('a' => int, ...)`.
  public function shape_and_iface(shape(...this::TShapeAndIface) $s): void {
    hh_expect<nothing>($s);
  }

  public function neither(shape(...this::TNeither) $s): void {
    hh_expect<nothing>($s);
  }
}

// The same unsatisfiable bounds on a type parameter do not collapse: they stay
// separate, so the element survives normalization as a residual.
function tparam<T as shape('a' => int, ...) as I>(shape(...T) $s): void {
  hh_expect_equivalent<T>($s);
}
