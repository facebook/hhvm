<?hh
/* Prototype  : string mb_regex_encoding([string $encoding])
 * Description: Returns the current encoding for regex as a string.
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test all listed encoding types from php.net to check all are known to function
 * NB: The strings passed are *NOT* necessarily encoded in the encoding passed to the function.
 * This test is purely to see whether the function recognises the encoding.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_regex_encoding() : usage variations ***\n";

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
foreach($encoding as $enc) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(mb_regex_encoding());
    var_dump(mb_regex_encoding($enc));
    var_dump(mb_regex_encoding());
    $iterator++;
}
echo "Done";
}
