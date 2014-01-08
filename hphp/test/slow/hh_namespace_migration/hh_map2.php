<?hh

// Test that the auto-import mechanism also kicks
// in when the "nameless" namespace construct is used.

namespace {

function main() {
  $m = Map {};
  var_dump($m->isEmpty());
}

main();

}
