<?hh
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass all encodings listed on php.net to test that function recognises them.
 * NB: The strings passed are *NOT* necessarily encoded in the encoding passed to the function.
 * This test is purely to see whether the function recognises the encoding.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_substr() : usage variations ***\n";

$encoding = vec['UCS-4',            /*1*/
                  'UCS-4BE',
                  'UCS-4LE',
                  'UCS-2',
                  'UCS-2BE',        /*5*/
                  'UCS-2LE',
                  'UTF-32',
                  'UTF-32BE',
                  'UTF-32LE',
                  'UTF-16',            /*10*/
                  'UTF-16BE',
                  'UTF-16LE',
                  'UTF-7',
                  'UTF7-IMAP',
                  'UTF-8',            /*15*/
                  'ASCII',
                  'EUC-JP',
                  'SJIS',
                  'eucJP-win',
                  'SJIS-win',        /*20*/
                  'ISO-2022-JP',
                  'JIS',
                  'ISO-8859-1',
                  'ISO-8859-2',
                  'ISO-8859-3',        /*25*/
                  'ISO-8859-4',
                  'ISO-8859-5',
                  'ISO-8859-6',
                  'ISO-8859-7',
                  'ISO-8859-8',        /*30*/
                  'ISO-8859-9',
                  'ISO-8859-10',
                  'ISO-8859-13',
                  'ISO-8859-14',
                  'ISO-8859-15',    /*35*/
                  'byte2be',
                  'byte2le',
                  'byte4be',
                  'byte4le',
                  'BASE64',            /*40*/
                  'HTML-ENTITIES',
                  '7bit',
                  '8bit',
                  'EUC-CN',
                  'CP936',            /*45*/
                  'HZ',
                  'EUC-TW',
                  'CP950',
                  'BIG-5',
                  'EUC-KR',            /*50*/
                  'UHC',
                  'ISO-2022-KR',
                  'Windows-1251',
                  'Windows-1252',
                  'CP866',            /*55*/
                  'KOI8-R'];        /*56*/



$iterator = 1;
$string_ascii = 'abc def';
//Japanese string encoded in UTF-8
$string_mb = base64_decode('44K/44OT44Ol44Os44O844OG44Kj44Oz44Kw44O744Oe44K344O844Oz44O744Kr44Oz44OR44OL44O8');

foreach($encoding as $enc) {
    echo "\n-- Iteration $iterator: $enc --\n";

    echo "-- ASCII String --\n";
    if (mb_substr($string_ascii, 1, 5, $enc)) {
        echo "Encoding $enc recognised\n";
    } else {
        echo "Encoding $enc not recognised\n";
    }

    echo "-- Multibyte String --\n";
    if (mb_substr($string_mb, 1, 5, $enc)) {
        echo "Encoding $enc recognised\n";
    } else {
        echo "Encoding $enc not recognised\n";
    }
}

echo "Done";
}
