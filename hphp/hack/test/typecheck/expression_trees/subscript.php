<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_subscript',
    'expression_tree_hack_arrays',
  )>>

// ===== Parameter-based tests (container passed as lambda parameter) =====

// Basic vec subscript
function test_subscript_vec(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx): ExampleInt ==> $arr[$idx]`;
}

// Basic dict subscript (string keys)
function test_subscript_dict(): void {
  ExampleDsl`
    (dict<ExampleString, ExampleInt> $map, ExampleString $key): ExampleInt ==>
      $map[$key]
  `;
}

// Dict subscript with int keys (matches BKSFrom::dictIntToType pattern)
function test_subscript_dict_int_key(): void {
  ExampleDsl`
    (dict<ExampleInt, ExampleString> $map, ExampleInt $key): ExampleString ==>
      $map[$key]
  `;
}

// Nested subscript: $arr[$i][$j]
function test_subscript_nested(): void {
  ExampleDsl`
    (vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j): ExampleInt ==>
      $arr[$i][$j]
  `;
}

// Nested dict+vec with mixed key types (string key -> vec -> int index)
function test_subscript_nested_mixed_keys(): void {
  ExampleDsl`
    (
      dict<ExampleString, vec<ExampleInt>> $m,
      ExampleString $k,
      ExampleInt $i,
    ): ExampleInt ==> $m[$k][$i]
  `;
}

// Subscript result used in arithmetic
function test_subscript_in_expression(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx): ExampleInt ==> $arr[$idx] + 1`;
}

// Subscript on property access result
function test_subscript_on_property(): void {
  ExampleDsl`(MyContainer $c, ExampleInt $idx): ExampleInt ==> $c->items[$idx]`;
}

abstract class MyContainer {
  public vec<ExampleInt> $items;
}

abstract class MyElement {
  public abstract function getVal(): ExampleInt;
}

// Subscript result as method call receiver: $arr[$idx]->method()
// Verifies visitSubscript + visitInstanceMethod compose correctly
function test_subscript_method_call(): void {
  ExampleDsl`
    (vec<MyElement> $arr, ExampleInt $idx): ExampleInt ==> $arr[$idx]->getVal()
  `;
}

// Keyset subscript (int keys)
function test_subscript_keyset_int(): void {
  ExampleDsl`(keyset<ExampleInt> $ks, ExampleInt $key): ExampleInt ==> $ks[$key]`;
}

// Keyset subscript (string keys)
function test_subscript_keyset_string(): void {
  ExampleDsl`
    (keyset<ExampleString> $ks, ExampleString $key): ExampleString ==> $ks[$key]
  `;
}

abstract class PropVec {
  public vec<ExampleInt> $items;
}

abstract class PropNestedVec {
  public vec<vec<ExampleInt>> $nested;
}

abstract class PropVecOfObj {
  public vec<PropVec> $children;
}

abstract class PropBaz {
  public vec<vec<ExampleInt>> $baz;
}

abstract class PropBarOfBaz {
  public vec<vec<PropBaz>> $bar;
}

abstract class PropDict {
  public dict<ExampleInt, ExampleInt> $data;
}

abstract class PropDeepVecOfObj {
  public vec<vec<PropVec>> $deep;
}

abstract class PropDeepVecOfNested {
  public vec<vec<PropNestedVec>> $deep;
}

// Property -> subscript read
function test_mixed_read_prop_sub(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $idx): ExampleInt ==> $obj->items[$idx]`;
}

// Property -> subscript -> subscript read
function test_mixed_read_prop_sub2(): void {
  ExampleDsl`
    (PropNestedVec $obj, ExampleInt $i, ExampleInt $j): ExampleInt ==>
      $obj->nested[$i][$j]
  `;
}

// Subscript -> property -> subscript read
function test_mixed_read_sub_prop_sub(): void {
  ExampleDsl`
    (vec<PropVec> $arr, ExampleInt $i, ExampleInt $j): ExampleInt ==>
      $arr[$i]->items[$j]
  `;
}

// Property -> subscript -> property -> subscript read
function test_mixed_read_prop_sub_prop_sub(): void {
  ExampleDsl`
    (PropVecOfObj $foo, ExampleInt $i, ExampleInt $j): ExampleInt ==>
      $foo->children[$i]->items[$j]
  `;
}

// Extreme: $foo->bar[$i][$j]->baz[$k][$m] — the original Dwayne Reeves example
function test_mixed_read_extreme(): void {
  ExampleDsl`
    (
      PropBarOfBaz $foo,
      ExampleInt $i,
      ExampleInt $j,
      ExampleInt $k,
      ExampleInt $m,
    ): ExampleInt ==> $foo->bar[$i][$j]->baz[$k][$m]
  `;
}

abstract class MyClassWithField {
  public ExampleInt $field;
}

// compound subscript assign: where an assignment is made to an object in a vec is supported,
// because field assignments are mutable, not copy-on-write operations, so this does not require
// an array reassignment
function test_mixed_chain_assign(): void {
  ExampleDsl`(vec<MyClassWithField> $arr, ExampleInt $val) ==> {
    $arr[0]->field = $val;
  }`;
}

// Basic assign
function test_subscript_assign(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx, ExampleInt $val) ==> {
    $arr[$idx] = $val;
  }`;
}

// Dict assign (string keys)
function test_subscript_assign_dict(): void {
  ExampleDsl`(
    dict<ExampleString, ExampleInt> $map,
    ExampleString $key,
    ExampleInt $val,
  ) ==> {
    $map[$key] = $val;
  }`;
}

// Dict assign (int keys)
function test_subscript_assign_dict_int_key(): void {
  ExampleDsl`(
    dict<ExampleInt, ExampleString> $map,
    ExampleInt $key,
    ExampleString $val,
  ) ==> {
    $map[$key] = $val;
  }`;
}

// Nested assign: $arr[$i][$j] = $val
function test_nested_subscript_assign(): void {
  ExampleDsl`(
    vec<vec<ExampleInt>> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $val,
  ) ==> {
    $arr[$i][$j] = $val;
  }`;
}

// Three-deep nested assign: $arr[$i][$j][$k] = $val
function test_3deep_subscript_assign(): void {
  ExampleDsl`(
    vec<vec<vec<ExampleInt>>> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $val,
  ) ==> {
    $arr[$i][$j][$k] = $val;
  }`;
}

// Nested dict+vec assign (string key -> vec -> int index)
function test_nested_subscript_assign_dict_string_key(): void {
  ExampleDsl`(
    dict<ExampleString, vec<ExampleInt>> $map,
    ExampleString $key,
    ExampleInt $idx,
    ExampleInt $val,
  ) ==> {
    $map[$key][$idx] = $val;
  }`;
}

// Property root, single subscript assign
function test_mixed_assign_prop_sub(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $idx, ExampleInt $val) ==> {
    $obj->items[$idx] = $val;
  }`;
}

// Property root, depth-2 subscript assign
function test_mixed_assign_prop_sub2(): void {
  ExampleDsl`(
    PropNestedVec $obj,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $val,
  ) ==> {
    $obj->nested[$i][$j] = $val;
  }`;
}

// Property root, dict subscript assign
function test_mixed_assign_prop_dict(): void {
  ExampleDsl`(PropDict $obj, ExampleInt $key, ExampleInt $val) ==> {
    $obj->data[$key] = $val;
  }`;
}

// Subscript -> property -> subscript assign
function test_mixed_assign_sub_prop_sub(): void {
  ExampleDsl`(
    vec<PropVec> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $val,
  ) ==> {
    $arr[$i]->items[$j] = $val;
  }`;
}

// Subscript^2 -> property -> subscript assign
function test_mixed_assign_sub2_prop_sub(): void {
  ExampleDsl`(
    vec<vec<PropVec>> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $val,
  ) ==> {
    $arr[$i][$j]->items[$k] = $val;
  }`;
}

// Subscript -> property -> subscript^2 assign
function test_mixed_assign_sub_prop_sub2(): void {
  ExampleDsl`(
    vec<PropNestedVec> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $val,
  ) ==> {
    $arr[$i]->nested[$j][$k] = $val;
  }`;
}

// Property -> subscript -> property -> subscript assign
function test_mixed_assign_prop_sub_prop_sub(): void {
  ExampleDsl`(
    PropVecOfObj $foo,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $val,
  ) ==> {
    $foo->children[$i]->items[$j] = $val;
  }`;
}

// Property -> sub^2 -> property -> subscript assign
function test_mixed_assign_prop_sub2_prop_sub(): void {
  ExampleDsl`(
    PropDeepVecOfObj $foo,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $val,
  ) ==> {
    $foo->deep[$i][$j]->items[$k] = $val;
  }`;
}

// Property -> sub^2 -> property -> sub^2 assign
function test_mixed_assign_prop_sub2_prop_sub2(): void {
  ExampleDsl`(
    PropDeepVecOfNested $foo,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $m,
    ExampleInt $val,
  ) ==> {
    $foo->deep[$i][$j]->nested[$k][$m] = $val;
  }`;
}

// Sub^2 -> property -> sub^2 (subscript-first variant of the deep mixed chain)
function test_mixed_assign_sub2_prop_sub2(): void {
  ExampleDsl`(
    vec<vec<PropNestedVec>> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $k,
    ExampleInt $m,
    ExampleInt $val,
  ) ==> {
    $arr[$i][$j]->nested[$k][$m] = $val;
  }`;
}

// Compound: assign where the RHS is a mixed-chain read
function test_mixed_assign_with_read(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $i, ExampleInt $j) ==> {
    $obj->items[$i] = $obj->items[$j];
  }`;
}

// Compound: assign with arithmetic on a mixed-chain read
function test_mixed_assign_with_arithmetic(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $i) ==> {
    $obj->items[$i] = $obj->items[$i] + 1;
  }`;
}

// Round-trip: assign through subscript, then read back at independent indices.
// Verifies subscript-read after subscript-assign typechecks cleanly and that
// the read result composes in arithmetic.
function test_subscript_assign_then_read(): void {
  ExampleDsl`(
    vec<ExampleInt> $arr,
    ExampleInt $i,
    ExampleInt $j,
    ExampleInt $val,
  ) ==> {
    $arr[$i] = $val;
    return $arr[$j] + $arr[$i];
  }`;
}

// Round-trip on a locally-declared vec literal: exercises vec-literal +
// local-var inference + subscript-assign + subscript-read together.
function test_local_vec_assign_then_read(): void {
  ExampleDsl`(ExampleInt $val, ExampleInt $i, ExampleInt $j) ==> {
    $x = vec[1, 2, 3];
    $x[$i] = $val;
    return $x[$j] + $x[$i];
  }`;
}
