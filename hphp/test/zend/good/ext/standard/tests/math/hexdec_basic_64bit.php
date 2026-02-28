<?hh <<__EntryPoint>> function main(): void {
echo "*** Testing hexdec() : basic functionality ***\n";

$values = vec[0x123abc,
                0x789DEF,
                0x7FFFFFFF,
                0x80000000,
                '0x123abc',
                '0x789DEF',
                '0x7FFFFFFF',
                '0x80000000',
                '0x123XYZABC',
                311015,
                '311015',
                31101.3,
                31.1013e5,
                011237,
                '011237',
                true,
                false,
                null];

foreach($values as $value) {
    $value__str = (string)($value);
    echo "\n-- hexdec $value__str --\n";
    var_dump(hexdec($value));
};
echo "===Done===";
}
