<?hh

function foo($x, $y)
:mixed{
  $x = (int)$x;
  $y = (int)$y;
  min($x, $y);
  echo max($x, $y);
}


<<__EntryPoint>>
function main_github_issue_5901() :mixed{
foo('30', '40');
}
