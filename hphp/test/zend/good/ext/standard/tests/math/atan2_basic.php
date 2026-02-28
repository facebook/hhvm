<?hh <<__EntryPoint>> function main(): void {
$valuesy = vec[23,
                -23,
                2.345e1,
                -2.345e1,
                0x17,
                027,
                "23",
                "23.45",
                "2.345e1",
                null,
                true,
                false];

$valuesx = vec[23,
                -23,
                2.345e1,
                -2.345e1,
                0x17,
                027,
                "23",
                "23.45",
                "2.345e1",
                null,
                true,
                false];

for ($i = 0; $i < count($valuesy); $i++) {
    for ($j = 0; $j < count($valuesx); $j++) {
        $res = atan2((float)$valuesy[$i], (float)$valuesx[$j]);
        echo "Y:".(string)$valuesy[$i]." X:".(string)$valuesx[$j]." ";
        var_dump($res);
    }
}
}
