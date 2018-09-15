<?hh

namespace Test;

// Test that HH\ImmSet is imported inside of a namespace.
function main() {
  $s = new ImmSet();
}


<<__EntryPoint>>
function main_hh_frozenset3() {
main();
}
