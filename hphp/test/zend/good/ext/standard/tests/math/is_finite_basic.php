<?hh <<__EntryPoint>> function main(): void {
$values = varray[234,
                -234,
                23.45e1,
                -23.45e1,
                0xEA,
                0352,
                "234",
                "234.5",
                "23.45e1",
                null,
                true,
                false,
                pow(0, -2),
                acos(1.01)];
;
for ($i = 0; $i < count($values); $i++) {
    $res = is_finite((float)$values[$i]);
    var_dump($res);
}
}
