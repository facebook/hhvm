<?hh <<__EntryPoint>> function main(): void {
$values = vec[111000111,
                011100000,
                100002001,
                '111000111',
                '011100000',
                '1111111111111111111111111111111',
                '10000000000000000000000000000000',
                '100002001',
                'abcdefg',
                311015,
                31101.3,
                31.1013e5,
                0x111ABC,
                011237,
                true,
                false,
                null];

for ($i = 0; $i < count($values); $i++) {
    $res = bindec($values[$i]);
    var_dump($res);
}
}
