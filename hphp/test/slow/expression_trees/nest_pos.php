<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et1 = ExampleDsl`
    1 + ${
      (() ==> {
        $et2 = ExampleDsl`1`;
          $pos = $et2->getEnclosingPos();
          var_dump($pos);
          return $et2;
      }
    )()}
  + 2`;
  $pos = $et1->getExprPos();
  invariant($et1->getEnclosingPos() === null, "Enclosing pos on top-level ET");
  var_dump($pos);
}
