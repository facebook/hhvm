<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_subscript')>>

// Wrong index type: string index on vec (expects int)
function test_subscript_wrong_index_type(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleString $key) ==> $arr[$key]`;
}

// Wrong index type on dict read: int index on dict<string, int> (Typing[4449])
function test_subscript_dict_wrong_key_read(): void {
  ExampleDsl`(dict<ExampleString, ExampleInt> $map, ExampleInt $key) ==> $map[$key]`;
}

// Wrong index type on int-keyed dict read: string index on dict<int, string> (Typing[4449])
function test_subscript_dict_int_key_wrong_key_read(): void {
  ExampleDsl`(dict<ExampleInt, ExampleString> $map, ExampleString $key) ==> $map[$key]`;
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
  ExampleDsl`(?dict<ExampleString, ExampleInt> $map, ExampleString $key) ==> $map[$key]`;
}

// Dict does not support array append syntax (Typing[4006])
function test_dict_append(): void {
  ExampleDsl`(dict<ExampleInt, ExampleString> $map, ExampleString $val) ==> {
    $map[] = $val;
  }`;
}

// Subscript coalesce: $arr[$idx] ?? $default — not yet supported
function test_subscript_coalesce(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx, ExampleInt $default) ==> $arr[$idx] ?? $default`;
}

// Subscript assignment: $arr[$idx] = $val — not yet supported
function test_subscript_assign(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx, ExampleInt $val) ==> {
    $arr[$idx] = $val;
  }`;
}

// Array append: $arr[] = $val — not yet supported
function test_array_append(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $val) ==> {
    $arr[] = $val;
  }`;
}

// Nested subscript assignment — not yet supported
function test_nested_subscript_assign(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j, ExampleInt $val) ==> {
    $arr[$i][$j] = $val;
  }`;
}

// Nested subscript postfix increment — not yet supported
function test_nested_subscript_postfix_increment(): void {
  ExampleDsl`(vec<vec<ExampleInt>> $arr, ExampleInt $i, ExampleInt $j) ==> {
    $arr[$i][$j]++;
  }`;
}
