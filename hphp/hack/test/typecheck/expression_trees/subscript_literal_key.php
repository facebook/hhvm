<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_subscript',
    'expression_tree_hack_arrays',
  )>>

// Regression coverage for literal-key container reads. The rest of the
// subscript tests index with a variable (`$map[$key]`), which does not
// exercise the virtualization of the key at all. A literal key does, so these
// catch changes that make `[]` resolve shape-only.
//
// String and int literal keys are the two that stay literal in the virtual
// expression, so both kinds are covered here. Shapes only exercise the string
// half: Hack requires a shape field name to be a single-quoted string or a
// class constant, so an int-keyed shape cannot be written down.

function test_dict_literal_key(): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $d = dict['a' => 1, 'b' => 2];
    return $d['a'];
  }`;
}

function test_keyset_literal_key(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $k = keyset['a', 'b'];
    return $k['a'];
  }`;
}

function test_vec_int_literal_key(): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $v = vec[1, 2, 3];
    return $v[0];
  }`;
}

function test_dict_int_literal_key(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $d = dict[1 => 'a', 2 => 'b'];
    return $d[1];
  }`;
}

function test_keyset_int_literal_key(): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $k = keyset[1, 2];
    return $k[1];
  }`;
}
