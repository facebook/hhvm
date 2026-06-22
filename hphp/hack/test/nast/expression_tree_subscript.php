<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_subscript')>>

function test_vec(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx]`;
}

function test_dict(): void {
  ExampleDsl`(dict<ExampleString, ExampleInt> $map, ExampleString $key) ==> $map[$key]`;
}

function test_nested(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j) ==> $arr[$i][$j]`;
}

function test_keyset(): void {
  ExampleDsl`(keyset<ExampleInt> $ks, ExampleInt $key) ==> $ks[$key]`;
}

abstract class PropVec {
  public vec<ExampleInt> $items;
}

function test_prop_subscript(): void {
  ExampleDsl`(PropVec $obj, ExampleInt $idx) ==> $obj->items[$idx]`;
}
