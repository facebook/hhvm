<?hh

// Test that the auto-import mechanism also kicks
// in when the "nameless" namespace construct is used.

namespace {

function main() {
  $s = Set {};
  var_dump($s->isEmpty());
}

main();

}
