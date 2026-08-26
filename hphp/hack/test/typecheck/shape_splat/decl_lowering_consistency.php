<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// These tests compare the two shape-splat decl-lowering implementations against
// each other to ensure consistency. A function signature is lowered by direct
// decl (direct_decl_smart_constructors.rs), while the identical hint written as
// an hh_expect_equivalent type argument is lowered by decl_hint.ml. If the two
// lowerings disagree, hh_expect_equivalent reports a mismatch, so any error
// below is a lowering divergence, not an expected type error.

type TOpen = shape('a' => int, ...);
type TClosed = shape('a' => int);

// (1) open base, explicit field, open tail: the open tail widens the base's
// 'a' to mixed. Both lowerings must agree on that widening.
function open_base_field_tail(): shape(...TOpen, 'y' => string, ...) {
  throw new Exception();
}
function test_open_base_field_tail(): void {
  hh_show(open_base_field_tail());
  hh_expect_equivalent<shape(...TOpen, 'y' => string, ...)>(
    open_base_field_tail(),
  );
}

// (2) all-splat open base (no explicit fields).
function open_base_only(): shape(...TOpen, ...) {
  throw new Exception();
}
function test_open_base_only(): void {
  hh_show(open_base_only());
  hh_expect_equivalent<shape(...TOpen, ...)>(open_base_only());
}

// (3) closed base, explicit field, open tail.
function closed_base_field_tail(): shape(...TClosed, 'y' => string, ...) {
  throw new Exception();
}
function test_closed_base_field_tail(): void {
  hh_show(closed_base_field_tail());
  hh_expect_equivalent<shape(...TClosed, 'y' => string, ...)>(
    closed_base_field_tail(),
  );
}

// (4) closed base, explicit field, closed.
function closed_base_field_closed(): shape(...TClosed, 'y' => string) {
  throw new Exception();
}
function test_closed_base_field_closed(): void {
  hh_show(closed_base_field_closed());
  hh_expect_equivalent<shape(...TClosed, 'y' => string)>(
    closed_base_field_closed(),
  );
}
