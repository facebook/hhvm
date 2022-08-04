<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

// There was an issue with rigid type variables and escaping type variables from lambda contexts
// that affected Expression Trees because we construct a type variable as the inferred type.
// This type variable was being lost at the end of the context, leading to inferring the
// nothing type accidentally

// This should throw an error, but it was not
function test_error(): (function(): nothing) {
  $x = () ==> {
    return ExampleDsl`1`;
  };
  return $x;
}

// This should not throw an error
function test(): (function(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>) {
  $x = () ==> {
    return ExampleDsl`1`;
  };
  return $x;
}

function test2(): (function(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>) {
  // The expression tree could be constructed outside of the lambda and this would be okay
  $y = ExampleDsl`1`;
  $x = () ==> {
    return $y;
  };
  return $x;
}

function test3(): (function(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>) {
  // The expression tree could be returned directly and this would also okay
  return () ==> {
    return ExampleDsl`1`;
  };
}
