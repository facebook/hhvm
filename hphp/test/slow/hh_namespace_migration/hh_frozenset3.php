<?hh

namespace Test;

// Test that HH\ImmSet is imported inside of a namespace.
function main() :mixed{
  $s = new ImmSet();
}


<<__EntryPoint>>
function main_hh_frozenset3() :mixed{
main();
}
