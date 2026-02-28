<?hh
/* Prototype  : string mb_strtolower(string $sourcestring [, string $encoding])
 * Description: Returns a lowercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass accented characters and Russian characters to check case conversion is correct
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strtolower() :  usage variations ***\n";

$uppers = dict['Basic Latin' => b'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
                'Characters With Accents' => base64_decode('w4DDgcOCw4PDhMOFw4bDh8OIw4nDisOLw4zDjcOOw4/DkMORw5LDk8OUw5XDlg=='),
                'Russian' => base64_decode('0JDQkdCS0JPQlNCV0JbQlw==')];
$lowers = dict['Basic Latin' => b'abcdefghijklmnopqrstuvwxyz',
                'Characters With Accents' => base64_decode('w6DDocOiw6PDpMOlw6bDp8Oow6nDqsOrw6zDrcOuw6/DsMOxw7LDs8O0w7XDtg=='),
                'Russian' => base64_decode('0LDQsdCy0LPQtNC10LbQtw==')];

foreach ($uppers as $lang => $sourcestring) {
    echo "\n-- $lang --\n";
    $a = mb_strtolower($sourcestring, 'utf-8');
    var_dump(base64_encode($a));
    if ($a == $lowers[$lang]) {
        echo "Correctly Converted\n";
    } else {
        echo "Incorrectly Converted\n";
    }
}

echo "Done";
}
