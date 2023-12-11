<?hh <<__EntryPoint>> function main(): void {
$values1 = vec[234,
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
                false];

$values2 = vec[2,
                -2,
                2.3e1,
                -2.3e1,
                0x2,
                02,
                "2",
                "2.3",
                "2.3e1",
                null,
                true,
                false];
for ($i = 0; $i < count($values1); $i++) {
    echo "\niteration ", $i, "\n";

    for ($j = 0; $j < count($values2); $j++) {
        $res = fmod((float)$values1[$i], (float)$values2[$j]);
        var_dump($res);
    }
}
}
