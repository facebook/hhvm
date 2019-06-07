<?hh

function foo($x, $y)
{
  $x = (int)$x;
  $y = (int)$y;
  min($x, $y);
  echo max($x, $y);
}


<<__EntryPoint>>
function main_github_issue_5901() {
foo('30', '40');
}
