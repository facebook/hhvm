<?hh
/* Prototype  : proto string base64_encode(string str)
 * Description: Encodes string using MIME base64 algorithm
 * Source code: ext/standard/base64.c
 * Alias to functions:
 */

/*
 * Test base64_encode with single byte values.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing base64_encode() : basic functionality ***\n";

$values = vec[
    "Hello World",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!%^&*(){}[]",
    "\n\t Line with control characters\r\n",
    "\xC1\xC2\xC3\xC4\xC5\xC6",
    "\75\76\77\78\79\80"
];

echo "\n--- Testing base64_encode() with binary string input ---\n";

$counter = 1;
foreach($values as $str) {
      echo "-- Iteration $counter --\n";

      $enc = base64_encode($str);
    $dec = base64_decode($enc);

    if ($dec != $str) {
        echo "TEST FAILED\n";
    } else {
        echo "TEST PASSED\n";
    }

      $counter ++;
}
echo "===Done===";
}
