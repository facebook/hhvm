<?hh

/*
 * This is unserializing an intentionally bad (but technically valid) string
 * to test that values that were overwritten during unserialization can still
 * be safely referenced with the R format.
 *
 * We may overwrite array elements (if have multiple entries with the same key)
 * and object properties (if we have multiple entries for the same property).
 *
 * If we overwrite these values, any reference to the element / property will
 * return the latest value, but we also have ref IDs for the old value as a
 * top-level item, so we can't destroy any values until the end.
 */

function test_overwritten_array_element($i) :mixed{
  var_dump(unserialize(
    'D:3:{' .
      's:1:"a";a:1:{' .
        'i:0;O:8:"stdClass":1:{' .
          's:1:"c";s:1:"d";' .
        '}' .
      '}' .
      's:1:"a";i:17;' .
      's:1:"b";R:'.$i.';' .
    '}'
  ));
}

function test_overwritten_property_value($i) :mixed{
  var_dump(unserialize(
    'O:8:"stdClass":3:{' .
      's:1:"a";a:1:{' .
        'i:0;O:8:"stdClass":1:{' .
          's:1:"c";s:1:"d";' .
        '}' .
      '}' .
      's:1:"a";i:17;' .
      's:1:"b";R:'.$i.';' .
    '}'
  ));
}

<<__EntryPoint>>
function main() :mixed{
  for ($i = 1; $i < 6; $i++) {
    print("\n=================================\n");
    print("Iteration $i\n");
    test_overwritten_array_element($i);
    test_overwritten_property_value($i);
  }
}
