<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $x = Code`1`;
  $et = Code`${ $x }`;
  $splices = $et->getSplices();

  foreach($splices as $key => $et) {
    $s = $et->visit(new Code());
    echo("$key => $s\n");
  }
}
