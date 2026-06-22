<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_subscript',
  )>>

// Wrong index type: string index on vec (expects int)
function test_subscript_wrong_index_type(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleString $key) ==> $arr[$key]`;
}

// Wrong index type on dict read: int index on dict<string, int> (Typing[4449])
function test_subscript_dict_wrong_key_read(): void {
  ExampleDsl`
    (dict<ExampleString, ExampleInt> $map, ExampleInt $key) ==> $map[$key]
  `;
}

// Wrong index type on int-keyed dict read: string index on dict<int, string> (Typing[4449])
function test_subscript_dict_int_key_wrong_key_read(): void {
  ExampleDsl`
    (dict<ExampleInt, ExampleString> $map, ExampleString $key) ==> $map[$key]
  `;
}

// Wrong return type: vec<ExampleInt> subscript yields ExampleInt, not ExampleString
function test_subscript_wrong_return_type_vec(): void {
  ExampleDsl`
    (vec<ExampleInt> $arr, ExampleInt $idx): ExampleString ==> $arr[$idx]
  `;
}

// Wrong return type: dict<_, ExampleInt> subscript yields ExampleInt, not ExampleString
function test_subscript_wrong_return_type_dict(): void {
  ExampleDsl`
    (dict<ExampleString, ExampleInt> $map, ExampleString $key): ExampleString ==>
      $map[$key]
  `;
}

// Wrong return type on nested subscript: vec<vec<ExampleInt>>[$i][$j] yields ExampleInt
function test_subscript_nested_wrong_return_type(): void {
  ExampleDsl`
    (vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j): ExampleString ==>
      $arr[$i][$j]
  `;
}

// Subscript on non-indexable type
function test_subscript_non_indexable(): void {
  ExampleDsl`(ExampleInt $x, ExampleInt $idx) ==> $x[$idx]`;
}

// Append syntax $arr[] as a read expression is not supported
function test_subscript_append_read(): void {
  ExampleDsl`(vec<ExampleInt> $arr) ==> $arr[]`;
}

// Nullable vec is not directly subscriptable
// (relevant to BKSFromNullable::vec auto-lifting patterns from D97586455)
function test_subscript_nullable_vec(): void {
  ExampleDsl`(?vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx]`;
}

// Nullable dict is not directly subscriptable
function test_subscript_nullable_dict(): void {
  ExampleDsl`
    (?dict<ExampleString, ExampleInt> $map, ExampleString $key) ==> $map[$key]
  `;
}

// Dict does not support array append syntax (Typing[4006])
function test_dict_append(): void {
  ExampleDsl`(dict<ExampleInt, ExampleString> $map, ExampleString $val) ==> {
    $map[] = $val;
  }`;
}

// Subscript coalesce: $arr[$idx] ?? $default - not yet supported
function test_subscript_coalesce(): void {
  ExampleDsl`
    (vec<ExampleInt> $arr, ExampleInt $idx, ExampleInt $default) ==>
      $arr[$idx] ?? $default
  `;
}

// Array append: $arr[] = $val - not yet supported
function test_array_append(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $val) ==> {
    $arr[] = $val;
  }`;
}

// Intermediate append: $arr[$i][] = $val — not yet supported, but caught by the
// chain walker rather than panicking. Verifies the `chain.iter().any(...)` guard
// in visitSubscriptAssign rejects None at any depth, not just the leaf.
function test_intermediate_append(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $val) ==> {
    $arr[$i][] = $val;
  }`;
}

// Nested subscript postfix increment — not yet supported
function test_nested_subscript_postfix_increment(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j) ==> {
    $arr[$i][$j]++;
  }`;
}

// Mixed-chain append: $obj->items[] = $val — not yet supported
function test_mixed_chain_append(): void {
  ExampleDsl`(MyContainerForErrors $obj, ExampleInt $val) ==> {
    $obj->items[] = $val;
  }`;
}

// Mixed-chain unop: $obj->items[$i]++ — not yet supported
function test_mixed_chain_unop(): void {
  ExampleDsl`(MyContainerForErrors $obj, ExampleInt $i) ==> {
    $obj->items[$i]++;
  }`;
}

// ClassGet root: `Foo::$prop[$i] = $val` — blocked at the ET level before
// chain walking runs (ET disallows static property access entirely). Documents
// that the chain walker doesn't have to handle ClassGet roots specially.
abstract class StaticHolder {
  public static vec<ExampleInt> $items = vec[];
}

function test_classget_root_subscript_assign(): void {
  ExampleDsl`(ExampleInt $i, ExampleInt $val) ==> {
    StaticHolder::$items[$i] = $val;
  }`;
}

// Dollardollar root: `$$[$i] = $val` — blocked at the ET level ($$
// is not supported in expression trees). Same documentation purpose as the
// ClassGet test above.
function test_dollardollar_root_subscript_assign(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $i, ExampleInt $val) ==> {
    $arr |> $$[$i] = $val;
  }`;
}

// Append at a non-leaf position: `$arr[][$j] = $val`. Complements
// `test_intermediate_append` (which has the append at the leaf) by exercising
// the `chain.iter().any(...)` guard for an intermediate None index.
function test_append_then_subscript(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $j, ExampleInt $val) ==> {
    $arr[][$j] = $val;
  }`;
}

// Compound assignment with subscript LHS: `$arr[$i] += 1`. The subscript
// branch must early-return once the compound-assignment error is pushed,
// otherwise the desugarer would silently drop the `+=` and emit a plain
// `visitSubscriptAssign(..., 1)`.
function test_compound_subscript_assignment(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $i) ==> {
    $arr[$i] += 1;
  }`;
}

// Subscript-assign in ET propagates the assigned value's type into subsequent
// reads: after assigning `ExampleString` into `vec<ExampleInt>`, `$arr[$j]`
// resolves to `ExampleStringOpType`, so `+ 1` fails for lack of `__plus`.
function test_subscript_assign_value_type_flows_to_subsequent_read(): void {
  ExampleDsl`(
    vec<ExampleInt> $arr,
    ExampleInt $i,
    ExampleString $widening_val,
    ExampleInt $j,
  ) ==> {
    $arr[$i] = $widening_val;
    return $arr[$j] + 1;
  }`;
}

abstract class MyContainerForErrors {
  public vec<ExampleInt> $items;
}
