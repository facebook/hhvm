<?hh

<<__EntryPoint>>
function main() :mixed{
  require __DIR__.'/require_constraint_repo_ifaces1.inc';
  require __DIR__.'/require_constraint_repo_ifaces2.inc';
  require __DIR__.'/require_constraint_repo_ifaces-classes.inc';

  $c1 = new C1();
  $c2 = new C2();
  echo 'Done', "\n";
}
