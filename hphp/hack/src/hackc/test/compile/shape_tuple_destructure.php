<?hh
// RUN: %hackc compile -vHack.Lang.AllowUnstableFeatures=1 %s | FileCheck %s
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Shape fields accessed by string key (ET), tuple entries by integer index (EI).
// CHECK-LABEL: .function {} {{.*}} shape_and_tuple() {
// CHECK:   QueryM 0 CGet ET:"x" Any
// CHECK:   QueryM 0 CGet EI:0 Any
function shape_and_tuple(): void {
  $s = shape('x' => 1);
  shape('x' => $a) = $s;
  $t = tuple(1);
  tuple($b) = $t;
}

// Optional field uses Idx (returns null on miss) instead of QueryM (throws).
// CHECK-LABEL: .function {} {{.*}} optional_field() {
// CHECK:   QueryM 0 CGet ET:"x" Any
// CHECK:   SetL $a
// CHECK:   String "y"
// CHECK:   Null
// CHECK:   Idx
// CHECK:   SetL $b
function optional_field(): void {
  $s = shape('x' => 1);
  shape('x' => $a, ?'y' => $b) = $s;
}

// Nested: outer shape field stored in temp, inner tuple indexes into it.
// CHECK-LABEL: .function {} {{.*}} nested() {
// CHECK:   QueryM 0 CGet ET:"x" Any
// CHECK:   SetL [[TMP:_[0-9]+]]
// CHECK:   BaseL [[TMP]] Warn Any
// CHECK:   QueryM 0 CGet EI:0 Any
// CHECK:   UnsetL [[TMP]]
function nested(): void {
  $s = shape('x' => tuple(1));
  shape('x' => tuple($a)) = $s;
}

// Wildcard _ accesses the field (proving it exists) then discards the value.
// CHECK-LABEL: .function {} {{.*}} wildcard() {
// CHECK:   BaseL {{.*}} Warn Any
// CHECK:   QueryM 0 CGet ET:"x" Any
// CHECK:   SetL $a
// CHECK:   BaseL {{.*}} Warn Any
// CHECK:   QueryM 0 CGet ET:"y" Any
// CHECK:   PopC
function wildcard(): void {
  $s = shape('x' => 1, 'y' => 2);
  shape('x' => $a, 'y' => _) = $s;
}

// Foreach: iterator value destructured via IterGetValue into temp.
// CHECK-LABEL: .function {} {{.*}} foreach_destructure() {
// CHECK:   IterGetValue
// CHECK:   PopL [[TMP:_[0-9]+]]
// CHECK:   QueryM 0 CGet ET:"x" Any
// CHECK:   UnsetL [[TMP]]
function foreach_destructure(): void {
  $v = vec[shape('x' => 1)];
  foreach ($v as shape('x' => $a)) {
    echo $a;
  }
}

async function gen_shape(): Awaitable<shape('x' => int)> {
  return shape('x' => 1);
}

// Await + shape destructuring: await evaluates first, result is destructured.
// CHECK-LABEL: .function {} {{.*}} await_shape_destructure() {{.*}} {
// CHECK:   FCallFuncD {{.*}} "gen_shape"
// CHECK:   Await
// CHECK:   PopL [[TMP:_[0-9]+]]
// CHECK:   BaseL [[TMP]] Warn Any
// CHECK:   QueryM 0 CGet ET:"x" Any
async function await_shape_destructure(): Awaitable<void> {
  shape('x' => $a) = await gen_shape();
}

// Shape literal on RHS: must be stashed in a temp before destructuring.
// CHECK-LABEL: .function {} {{.*}} shape_literal_rhs() {
// CHECK:   PopL [[TMP:_[0-9]+]]
// CHECK:   BaseL [[TMP]] Warn Any
// CHECK:   QueryM 0 CGet ET:"x" Any
function shape_literal_rhs(): void {
  shape('x' => $a) = shape('x' => 10);
}

async function gen_tuple(): Awaitable<(int, string)> {
  return tuple(1, 'one');
}

// Await + tuple destructuring.
// CHECK-LABEL: .function {} {{.*}} await_tuple_destructure() {{.*}} {
// CHECK:   FCallFuncD {{.*}} "gen_tuple"
// CHECK:   Await
// CHECK:   PopL [[TMP:_[0-9]+]]
// CHECK:   BaseL [[TMP]] Warn Any
// CHECK:   QueryM 0 CGet EI:0 Any
async function await_tuple_destructure(): Awaitable<void> {
  tuple($a, $b) = await gen_tuple();
}
