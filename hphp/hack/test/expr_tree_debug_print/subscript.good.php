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
