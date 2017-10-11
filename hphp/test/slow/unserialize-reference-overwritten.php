<?hh
/**
 * This is unserializing an intentionally bad (but technically valid) string
 * to test that values that were overwritten during unserialization can still
 * be safely referenced with the R format.
 * Property 'a' of the outer stdClass is first set to an array with another
 * stdClass in it, then overwritten with a 1; then property 'b' of the outer
 * stdClass resurrects the inner stdClass value.
 */

var_dump(unserialize(
  'O:8:"stdClass":3:{' .
    's:1:"a";a:1:{' .
      'i:0;O:8:"stdClass":1:{' .
        's:1:"a";s:1:"a";' .
      '}' .
    '}' .
    's:1:"a";i:1;' .
    's:1:"b";R:3;' .
  '}'
));
