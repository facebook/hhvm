<?hh
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test how mb_substr() behaves when passed a range of integers as $length argument
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_substr() : usage variations ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'+Is an English string'; //21 chars

//Japanese string, 21 characters
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

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
    $a = mb_substr($string_ascii, 1, $i);
    var_dump(base64_encode($a));
    echo "--Multibyte String --\n";
    $b = mb_substr($string_mb, 1, $i, 'UTF-8');
    if (strlen($a) == mb_strlen($b, 'UTF-8')) { // should return same length
        var_dump(base64_encode($b));
    } else {
        echo "Difference in length of ASCII string and multibyte string\n";
    }

}

echo "Done";
}
