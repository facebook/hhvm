<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function from(string $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  throw new Exception();
}

function f(): void {
    $hack_bool_value = true;
    $bar = "";
    $foo = "";
    $bug = ExampleDsl`
      true ? ${$hack_bool_value ? from($foo) : from($bar)} : 'baz'
//         ^ hover-at-caret
    `;
}
