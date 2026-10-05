<?hh
// RUN: %hackc compile -vHack.Lang.AllowUnstableFeatures=1 %s | FileCheck %s
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// A shape with no splats is a plain dict literal: constant-folded to a static
// Dict, no runtime construction.
// CHECK-LABEL: .function {} {{.*}} no_splat() {
// CHECK:   Dict @A_
// CHECK-NOT: AddElemC
function no_splat(): dict<string, mixed> {
  return shape('x' => 1, 'y' => 2);
}

// A shape containing a splat is built at runtime: seed an accumulator dict in a
// local, add literal fields with AddElemC, and copy each splat operand key/value
// with an iterator loop. Source order gives rightmost-wins.
// CHECK-LABEL: .function {} {{.*}} field_then_splat({{.*}}) {
// CHECK:   NewDictArray 0
// CHECK:   PopL [[ACC:_[0-9]+]]
// Leading literal field 'p' => 1:
// CHECK:   PushL [[ACC]]
// CHECK:   String "p"
// CHECK:   Int 1
// CHECK:   AddElemC
// CHECK:   PopL [[ACC]]
// Splat operand $a iterated key-by-key into the accumulator:
// CHECK:   CGetL $a
// CHECK:   Dup
// CHECK:   IsTypeC Dict
// CHECK:   JmpNZ [[DICT_OK:L[0-9]+]]
// CHECK:   Dict @A_
// CHECK:   ThrowAsTypeStructException Typehint
// CHECK:   [[DICT_OK]]:
// CHECK-NOT: IterBase
// CHECK:   PopL [[BASE:_[0-9]+]]
// CHECK:   IterInit <WithKeys> {{[0-9]+}} [[BASE]] [[END:L[0-9]+]]
// CHECK:   [[TOP:L[0-9]+]]:
// CHECK:   IterGetValue <WithKeys> {{[0-9]+}} [[BASE]]
// CHECK:   PopL [[VAL:_[0-9]+]]
// CHECK:   IterGetKey <WithKeys> {{[0-9]+}} [[BASE]]
// CHECK:   PopL [[KEY:_[0-9]+]]
// CHECK:   PushL [[ACC]]
// CHECK:   CGetL [[KEY]]
// CHECK:   CGetL [[VAL]]
// CHECK:   AddElemC
// CHECK:   PopL [[ACC]]
// CHECK:   IterNext <WithKeys> {{[0-9]+}} [[BASE]] [[TOP]]
// CHECK:   PushL [[ACC]]
function field_then_splat(dict<string, mixed> $a): dict<string, mixed> {
  return shape('p' => 1, ...$a);
}

// A lone splat still builds an accumulator (a fresh copy), never a static dict.
// CHECK-LABEL: .function {} {{.*}} only_splat({{.*}}) {
// CHECK:   NewDictArray 0
// CHECK:   IterInit
// CHECK:   AddElemC
function only_splat(dict<string, mixed> $a): dict<string, mixed> {
  return shape(...$a);
}

// Property initializers use a synthetic 86pinit method, which must advertise
// the iterator slot used by the splat loop.
// CHECK-LABEL: .class {{.*}} ShapeSplatProperty
// CHECK: .method {{.*}} 86pinit() {
// CHECK-NEXT: .numiters 1;
class ShapeSplatProperty {
  public shape('x' => int, 'y' => string) $value =
    shape(...shape('x' => 1), 'y' => 'property');
}
