<?hh <<__EntryPoint>> function main(): void {
$values = vec[23,
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

echo "\n LOG tests...no base\n";
for ($i = 0; $i < count($values); $i++) {
    $res = log((float)$values[$i]);
    var_dump($res);
}

echo "\n LOG tests...base\n";
for ($i = 0; $i < count($values); $i++) {
    $res = log((float)$values[$i], 4.0);
    var_dump($res);
}
}
