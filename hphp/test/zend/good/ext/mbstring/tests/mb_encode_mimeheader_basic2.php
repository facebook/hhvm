<?hh
/* Prototype  : string mb_encode_mimeheader(string $str [, string $charset
 * [, string $transfer-encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?=
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test mb_encode_header() with different strings
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_encode_mimeheader() : basic2 ***\n";

//All strings are the same when displayed in their respective encodings
$sjis_string = base64_decode('k/qWe4zqg2WDTINYg2eCxYK3gUIwMTIzNIJUglWCVoJXgliBQg==');
$jis_string = base64_decode('GyRCRnxLXDhsJUYlLSU5JUgkRyQ5ISMbKEIwMTIzNBskQiM1IzYjNyM4IzkhIxsoQg==');
$euc_jp_string = base64_decode('xvzL3LjspcalraW5pcikx6S5oaMwMTIzNKO1o7ajt6O4o7mhow==');

$inputs = dict['SJIS' => $sjis_string,
                'JIS' => $jis_string,
                'EUC_JP' => $euc_jp_string];

foreach ($inputs as $lang => $input) {
    echo "\nLanguage: $lang\n";
    echo "-- Base 64: --\n";
    mb_internal_encoding($lang);
    $outEncoding = "UTF-8";
    var_dump(mb_encode_mimeheader($input, $outEncoding, 'B'));
    echo "-- Quoted-Printable --\n";
    var_dump(mb_encode_mimeheader($input, $outEncoding, 'Q'));
}

echo "Done";
}
