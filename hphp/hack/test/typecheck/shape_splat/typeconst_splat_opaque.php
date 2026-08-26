<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A type constant spread in a shape. An abstract one is opaque and behaves like
// a rigid type parameter (it localizes to a `Tgeneric` whose bound is the
// projection) so it already survives normalization as a residual element.
//
// See wf_splat_tparam_typeconst_bound.php for the neighbouring case of a type
// parameter *bounded by* a type constant, which does work.

abstract class A {
  abstract const type TSAs as shape('a' => int, ...);
  abstract const type TSOther as shape('b' => bool, ...);
  abstract const type TSPlain;

  const type TSEq = shape('a' => int);

  // A concrete type constant defined by splatting other type constants.
  const type TSBoth = shape(...this::TSAs, ...this::TSOther);

  const type TSBothPlain = shape(...this::TSAs, ...this::TSPlain);

  // Opaque, so it stays a residual element.
  public function bounded(shape(...this::TSAs) $s): void {
    hh_expect_equivalent<this::TSAs>($s);
  }

  public function row(shape(...this::TSAs, 'x' => int) $s): void {
    hh_expect_equivalent<shape(...this::TSAs, 'x' => int)>($s);
  }

  // The bound guarantees 'a'. Reaching it needs one indirection: the tpenv
  // gives `A::TSAs`, another projection, and the shape is only visible via its
  // concrete supertypes.
  public function read(shape(...this::TSAs, 'x' => int) $s): void {
    hh_expect<int>($s['a']);
  }

  // REJECT: no `as` clause, so this is `mixed`. `B` instantiates it as `int`.
  public function unbounded(shape(...this::TSPlain) $s): void {}

  // Concrete: expands.
  public function concrete(shape(...this::TSEq) $s): void {
    hh_expect_equivalent<shape('a' => int)>($s);
  }

  // A type constant that is itself built from splatted type constants.
  public function both(shape(...this::TSBoth) $s): void {
    hh_expect_equivalent<shape(...this::TSAs, ...this::TSOther)>($s);
  }

  public function both_plain(shape(...this::TSBothPlain) $s): void {
    hh_expect_equivalent<shape(...this::TSAs, ...this::TSPlain)>($s);
  }
}

class B extends A {
  const type TSAs = shape('a' => int, 'extra' => string);
  const type TSOther = shape('b' => bool);
  const type TSPlain = int;
}

// At a concrete receiver the projections resolve, so `B::TSBoth` is the merge of
// `B::TSAs` and `B::TSOther`.
function use_concrete(
  B $b,
  shape('a' => int, 'extra' => string, 'b' => bool) $s,
): void {
  $b->both($s);
}
