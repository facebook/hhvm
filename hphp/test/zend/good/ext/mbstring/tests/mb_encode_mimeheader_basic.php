<?hh
/* Prototype  : string mb_encode_mimeheader
 * (string $str [, string $charset [, string $transfer-encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?=
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test basic functionality of mb_encode_mimeheader with different strings.
 * For the below strings:
 * 'English' is ASCII only, 'Japanese' has no ASCII characters and 'Greek' is mixed.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_encode_mimeheader() : basic ***\n";

$english = dict['English' => 'This is an English string. 0123456789'];
$nonEnglish = dict['Japanese' => base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CC'),
                'Greek' => base64_decode('zpHPhc+Ez4wgzrXOr869zrHOuSDOtc67zrvOt869zrnOus+MIM66zrXOr868zrXOvc6/LiAwMTIzNDU2Nzg5Lg==')];

foreach ($english as $lang => $input) {
    echo "\nLanguage: $lang\n";
    echo "-- Base 64: --\n";
    var_dump(mb_encode_mimeheader($input, 'UTF-8', 'B'));
    echo "-- Quoted-Printable --\n";
    var_dump(mb_encode_mimeheader($input, 'UTF-8', 'Q'));
}

mb_internal_encoding('utf-8');

foreach ($nonEnglish as $lang => $input) {
    echo "\nLanguage: $lang\n";
    echo "-- Base 64: --\n";
    var_dump(mb_encode_mimeheader($input, 'UTF-8', 'B'));
    echo "-- Quoted-Printable --\n";
    var_dump(mb_encode_mimeheader($input, 'UTF-8', 'Q'));
}

echo "Done";
}
