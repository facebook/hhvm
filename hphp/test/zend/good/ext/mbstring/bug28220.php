<?hh <<__EntryPoint>> function main(): void {
$coderange = vec[
    range(0x0000, 0x1fff),
    range(0xff60, 0xff9f)
];


foreach ($coderange as $r) {
    $ng = 0;
    foreach ($r as $c) {
        if (mb_strwidth(pack('N1', $c), 'UCS-4BE') != 2) {
            $ng++;
        }
    }
    echo "$ng\n";
}
}
