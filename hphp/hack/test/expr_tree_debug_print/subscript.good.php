<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_subscript')>>

function test_vec(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx]`;
}

function test_dict(): void {
  ExampleDsl`(dict<ExampleString, ExampleInt> $map, ExampleString $key) ==> $map[$key]`;
}

function test_dict_int_key(): void {
  ExampleDsl`(dict<ExampleInt, ExampleString> $map, ExampleInt $key) ==> $map[$key]`;
}

function test_nested(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j) ==> $arr[$i][$j]`;
}

function test_keyset_int(): void {
  ExampleDsl`(keyset<ExampleInt> $ks, ExampleInt $key) ==> $ks[$key]`;
}

function test_keyset_string(): void {
  ExampleDsl`(keyset<ExampleString> $ks, ExampleString $key) ==> $ks[$key]`;
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

function test_mixed_read_prop_sub(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $idx) ==> $obj->items[$idx]`;
}

function test_mixed_read_prop_sub2(): void {
  ExampleDsl`(PropNestedVec $obj, ExampleInt $i, ExampleInt $j) ==> $obj->nested[$i][$j]`;
}

function test_mixed_read_sub_prop_sub(): void {
  ExampleDsl`(vec<PropVec> $arr, ExampleInt $i, ExampleInt $j) ==> $arr[$i]->items[$j]`;
}

function test_mixed_read_prop_sub_prop_sub(): void {
  ExampleDsl`(PropVecOfObj $foo, ExampleInt $i, ExampleInt $j) ==> $foo->children[$i]->items[$j]`;
}

function test_mixed_read_extreme(): void {
  ExampleDsl`(PropBarOfBaz $foo, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $m) ==> $foo->bar[$i][$j]->baz[$k][$m]`;
}

function test_subscript_assign(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx, ExampleInt $val) ==> {
    $arr[$idx] = $val;
  }`;
}

function test_subscript_assign_nested(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $val) ==> {
    $arr[$i][$j] = $val;
  }`;
}

// Property root, single subscript assign — confirms property access flows
// through as the root expression of visitSubscriptAssign and the property
// l-value mechanism handles the writeback.
function test_mixed_assign_prop_sub(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $idx, ExampleInt $val) ==> {
    $obj->items[$idx] = $val;
  }`;
}

function test_mixed_assign_prop_sub2(): void {
  ExampleDsl`(PropNestedVec $obj, ExampleInt $i, ExampleInt $j, ExampleInt $val) ==> {
    $obj->nested[$i][$j] = $val;
  }`;
}

function test_mixed_assign_prop_dict(): void {
  ExampleDsl`(PropDict $obj, ExampleInt $key, ExampleInt $val) ==> {
    $obj->data[$key] = $val;
  }`;
}

function test_mixed_assign_sub_prop_sub(): void {
  ExampleDsl`(vec<PropVec> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $val) ==> {
    $arr[$i]->items[$j] = $val;
  }`;
}

function test_mixed_assign_sub2_prop_sub(): void {
  ExampleDsl`(vec<vec<PropVec>> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $val) ==> {
    $arr[$i][$j]->items[$k] = $val;
  }`;
}

function test_mixed_assign_sub_prop_sub2(): void {
  ExampleDsl`(vec<PropNestedVec> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $val) ==> {
    $arr[$i]->nested[$j][$k] = $val;
  }`;
}

function test_mixed_assign_prop_sub_prop_sub(): void {
  ExampleDsl`(PropVecOfObj $foo, ExampleInt $i, ExampleInt $j, ExampleInt $val) ==> {
    $foo->children[$i]->items[$j] = $val;
  }`;
}

function test_mixed_assign_prop_sub2_prop_sub(): void {
  ExampleDsl`(PropDeepVecOfObj $foo, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $val) ==> {
    $foo->deep[$i][$j]->items[$k] = $val;
  }`;
}

function test_mixed_assign_prop_sub2_prop_sub2(): void {
  ExampleDsl`(PropDeepVecOfNested $foo, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $m, ExampleInt $val) ==> {
    $foo->deep[$i][$j]->nested[$k][$m] = $val;
  }`;
}

function test_mixed_assign_sub2_prop_sub2(): void {
  ExampleDsl`(vec<vec<PropNestedVec>> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $k, ExampleInt $m, ExampleInt $val) ==> {
    $arr[$i][$j]->nested[$k][$m] = $val;
  }`;
}

function test_mixed_assign_with_read(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $i, ExampleInt $j) ==> {
    $obj->items[$i] = $obj->items[$j];
  }`;
}

function test_mixed_assign_with_arithmetic(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $i) ==> {
    $obj->items[$i] = $obj->items[$i] + 1;
  }`;
}
