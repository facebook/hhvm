<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public ExampleInt $prop;
}

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, string> $splice_var): void {
  ExampleDsl`(MyClass $x) ==> {
    $z = ${ $splice_var };
    $x->$z;
  }`;
}
