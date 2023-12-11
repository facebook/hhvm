<?hh <<__EntryPoint>> function main(): void {
$input = vec["foo", "bar", "baz", "grldsajkopallkjasd"];
foreach($input as $i) {
    printf("%u\n", crc32($i));
}
}
