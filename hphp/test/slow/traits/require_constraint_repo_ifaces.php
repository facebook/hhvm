<?hh
require __DIR__.'/require_constraint_repo_ifaces.inc';

trait DYI implements IDY {}

class C {
  use MyT; // requires IDY
  use DYI; // provides IDY
}

function main() {
  $c = new C();
  echo 'Done', "\n";
}
main();
