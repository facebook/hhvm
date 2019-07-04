<?hh
require __DIR__.'/require_constraint_repo_ifaces1.inc';
require __DIR__.'/require_constraint_repo_ifaces2.inc';

class C1 {
  use ChildRequiresT;
  use ChildProvidesT;
}

class C2 extends Super {
  use ChildRequiresT;
}

<<__EntryPoint>>
function main() {
  $c1 = new C1();
  $c2 = new C2();
  echo 'Done', "\n";
}
