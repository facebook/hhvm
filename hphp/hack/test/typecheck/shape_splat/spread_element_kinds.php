<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A shape can spread three kinds of thing whose fields are not known here: a
// type parameter, an opaque newtype, and a type constant. All three are handled
// the same way by the code that works out field ranges and orders parameters,
// but only type parameters can be built by hand in typingCornersTest, because
// the other two need real declarations. So they are covered here.

newtype Row as shape('a' => int, ...) = shape('a' => int, 'extra' => string);

abstract class C {
  abstract const type TRow as shape('a' => int, ...);

  // -- Type constant as the spread element ---------------------------------

  // ACCEPT: the bound guarantees 'a'.
  public function tconst_read(shape(...this::TRow) $s): void {
    hh_expect<int>($s['a']);
  }

  // ACCEPT: the same row on both sides.
  public function tconst_same(
    shape(...this::TRow, 'q' => int) $s,
  ): shape(...this::TRow, 'q' => int) {
    return $s;
  }

  // REJECT: the bound says 'a' is an int, and this asks for a string.
  public function tconst_field(
    shape(...this::TRow) $s,
  ): shape(...this::TRow, 'a' => string) {
    return $s;
  }

  // A type parameter bounded BY the type constant, so one spread element's
  // bound mentions another kind of spread element.
  // ACCEPT: T is below the type constant's row.
  public function tconst_bounds_param<T as shape(...this::TRow)>(
    shape(...T, 'q' => int) $s,
  ): shape(...T, 'q' => int) {
    return $s;
  }
}

// -- Newtype as the spread element -----------------------------------------

// ACCEPT: the newtype's bound guarantees 'a'.
function newtype_read(shape(...Row) $s): void {
  hh_expect<int>($s['a']);
}

// ACCEPT: the same row on both sides.
function newtype_same(shape(...Row, 'q' => int) $s): shape(...Row, 'q' => int) {
  return $s;
}

// REJECT: the bound says 'a' is an int, and this asks for a string.
function newtype_field(shape(...Row) $s): shape(...Row, 'a' => string) {
  return $s;
}

// A type parameter bounded by the newtype, so a parameter's bound mentions a
// newtype spread element.
// ACCEPT: T is below the newtype's row.
function newtype_bounds_param<T as shape(...Row)>(
  shape(...T, 'q' => int) $s,
): shape(...T, 'q' => int) {
  return $s;
}

// REJECT: nothing says a newtype row is below a parameter's row.
function newtype_not_below_param<T as shape(...)>(
  shape(...Row, 'q' => int) $s,
): shape(...T, 'q' => int) {
  return $s;
}
