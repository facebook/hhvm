<?hh


class Str {
/**
 * Test whether a string has a specific suffix
 */
public static function endsWith(string $str, string $test): bool {
  return strlen($str) >= strlen($test) &&
    substr_compare($str, $test, -strlen($test), strlen($test)) === 0;
}
}

for ($i = 0; $i < 12; $i++) {
  var_dump(Str::endsWith(str_repeat('x', $i), "bar"));
}
