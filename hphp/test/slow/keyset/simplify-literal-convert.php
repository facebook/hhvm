<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// Make sure the simplifier doesn't crash on trying to simplify invalid
// conversions to keyset.

function main() :mixed{
  // These should all fail and not be simplified away at JIT time
  try {
    var_dump(keyset(dict[1 => 'a', 2 => 100, 3 => false]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  try {
    var_dump(keyset(vec['a', 100, false]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  try {
    var_dump(keyset(vec['a', 100, false]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  try {
    var_dump(keyset(dict[1 => 'a', 2 => 100, 3 => false]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  // These should all succeed and be simplified away at JIT time
  var_dump(keyset(dict[1 => 'a', 2 => 100, 3 => 'b']));
  var_dump(keyset(vec['a', 100, 'b']));
  var_dump(keyset(vec['a', 100, 'b']));
  var_dump(keyset(dict[1 => 'a', 2 => 100, 3 => 'b']));
}


<<__EntryPoint>>
function main_simplify_literal_convert() :mixed{
main();
}
