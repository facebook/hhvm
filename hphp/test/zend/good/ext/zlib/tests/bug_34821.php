<?hh
<<__EntryPoint>> function main(): void {
// test 50 bytes to 50k
$b = vec[
    50,
    500,
    5000,
    50000,
//  1000000, // works, but test would take too long
];

$s = '';
$i = 0;

foreach ($b as $size) {
    do {
        $s .= chr(rand(0,255));
    } while (++$i < $size);
    var_dump($s === gzinflate(gzdeflate($s)));
    var_dump($s === gzuncompress(gzcompress($s)));
    var_dump($s === gzinflate(substr(gzencode($s), 10, -8)));
}
}
