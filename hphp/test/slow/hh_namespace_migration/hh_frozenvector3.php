<?hh

namespace Test;

// Test that HH\ImmVector is auto-imported in a namespace.
function main() :mixed{
  $s = new ImmVector();
}


<<__EntryPoint>>
function main_hh_frozenvector3() :mixed{
main();
}
