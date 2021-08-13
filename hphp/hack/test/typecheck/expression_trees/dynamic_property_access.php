<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  public ExampleInt $prop;
}

function test(ExprTree<Code, Code::TAst, string> $splice_var): void {
  Code`(MyClass $x) ==> {
    $z = ${ $splice_var };
    $x->$z;
  }`;
}
