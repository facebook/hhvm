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
