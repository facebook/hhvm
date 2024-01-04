<?hh
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test how mb_substr() behaves when passed a range of integers as $start argument
 */
<<__EntryPoint>>
function main(): void {
  echo "*** Testing mb_substr() : usage variations ***\n";

  mb_internal_encoding('UTF-8');

  $string_ascii = b'+Is an English string'; //21 chars

  $string_mb = base64_decode(
    '5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=',
  ); //21 chars

  /*
   * Loop through integers as multiples of ten for $offset argument
   * 60 is larger than *BYTE* count for $string_mb
   */
  for ($i = -60; $i <= 60; $i += 10) {
    try {
      if ($a || $b) {
        $a = null;
        $b = null;
      }
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
    echo "\n**-- Offset is: $i --**\n";
    echo "-- ASCII String --\n";
    $a = mb_substr($string_ascii, $i, 4);
    if ($a !== false) {
      var_dump(bin2hex($a));
    } else {
      var_dump($a);
    }
    echo "--Multibyte String --\n";
    $b = mb_substr($string_mb, $i, 4, 'UTF-8');
    if (strlen($a) == mb_strlen($b, 'UTF-8')) { // should return same length
      var_dump(bin2hex($b));
    } else {
      echo "Difference in length of ASCII string and multibyte string\n";
    }

  }

  echo "Done";
}
