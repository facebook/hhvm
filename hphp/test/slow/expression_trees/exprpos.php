<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`1`;
  $pos = $et->getExprPos();
  if ($pos !== null) {
    $output = sprintf("%s: (%d, %d)-(%d, %d)\n", $pos['path'], $pos['start_line'], $pos['start_column'], $pos['end_line'], $pos['end_column']);
    echo($output);
  } else {
    echo("Position is null\n");
  }
}
