<?hh <<__EntryPoint>> function main(): void {
$values = vec[1234.5678,
                -1234.5678,
                1234.6578e4,
                -1234.56789e4,
                0x1234CDEF,
                02777777777,
                "123456789",
                "123.456789",
                "12.3456789e1",
                null,
                true,
                false];

echo "\n number_format tests.....default\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i]);
    var_dump($res);
}

echo "\n number_format tests.....with two dp\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2);
    var_dump($res);
}

echo "\n number_format tests.....English format\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2, '.', ' ');
    var_dump($res);
}

echo "\n number_format tests.....French format\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2, ',' , ' ');
    var_dump($res);
}
}
