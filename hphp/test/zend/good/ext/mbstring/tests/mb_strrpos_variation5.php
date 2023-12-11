<?hh
/* Prototype  : int mb_strrpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of last occurrence of a string within another
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Testing deprecated behaviour where third argument can be $encoding
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strrpos() : usage variations ***\n";

$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');
$needle_mb = base64_decode('44CC');

$stringh = <<<END
utf-8
END;

$inputs = dict['Double Quoted String' => "utf-8",
                'Single Quoted String' => 'utf-8',
                'Heredoc' => $stringh];
foreach ($inputs as $type => $input) {
    echo "\n-- $type --\n";
    echo "-- With fourth encoding argument --\n";
    var_dump(mb_strrpos($string_mb, $needle_mb, $input, 'utf-8'));
    echo "-- Without fourth encoding argument --\n";
    var_dump(mb_strrpos($string_mb, $needle_mb, $input));
}

echo "Done";
}
