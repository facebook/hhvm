<?hh

// Test that FS is immutable.
<<__EntryPoint>>
function main() :mixed{
  $fs = ImmSet {1, 2, 3};
  $fs[] = 10;
}
