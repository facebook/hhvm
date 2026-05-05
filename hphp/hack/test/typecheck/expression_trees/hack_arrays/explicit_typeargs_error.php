<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

// Explicit type arguments on collection literals are rejected inside
// expression trees. See D97993202 reviewer comment from dreeves: until a HIP
// decides how typeargs should desugar, the syntax must fail at the boundary.

function test_vec_explicit_typearg(): void {
  ExampleDsl`vec<ExampleInt>[1, 2, 3]`;
}

function test_vec_explicit_typearg_empty(): void {
  ExampleDsl`vec<ExampleInt>[]`;
}

function test_dict_explicit_typeargs(): void {
  ExampleDsl`dict<ExampleString, ExampleInt>["a" => 1]`;
}

function test_dict_explicit_typeargs_empty(): void {
  ExampleDsl`dict<ExampleString, ExampleInt>[]`;
}

function test_keyset_explicit_typearg(): void {
  ExampleDsl`keyset<ExampleInt>[1, 2, 3]`;
}

function test_keyset_explicit_typearg_empty(): void {
  ExampleDsl`keyset<ExampleInt>[]`;
}

// Nested: both the outer vec<...> and the inner vec<...> should error
// (desugaring recurses through values).
function test_nested_explicit_typeargs(): void {
  ExampleDsl`vec<vec<ExampleInt>>[vec<ExampleInt>[1, 2]]`;
}

// Inside a lambda body — error position should still resolve.
function test_explicit_typearg_in_lambda(): void {
  ExampleDsl`() ==> vec<ExampleInt>[1, 2]`;
}
