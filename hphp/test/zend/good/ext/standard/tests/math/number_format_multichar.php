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

echo " number_format tests.....multiple character decimal point\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2, '&#183;', ' ');
    var_dump($res);
}

echo "\n number_format tests.....multiple character thousand separator\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2, '.' , '&thinsp;');
    var_dump($res);
}

echo "\n number_format tests.....multiple character decimal and thousep\n";
for ($i = 0; $i < count($values); $i++) {
    $res = number_format((float)$values[$i], 2, '&#183;' , '&thinsp;');
    var_dump($res);
}
}
