<?hh

namespace Test;

// Test that HH\Pair is auto-imported in a namespace.
function main() {
  $s = Pair {1, 2};
}


<<__EntryPoint>>
function main_hh_pair3() {
main();
}
