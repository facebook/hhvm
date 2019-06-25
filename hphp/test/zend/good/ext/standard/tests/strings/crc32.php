<?hh <<__EntryPoint>> function main(): void {
$input = array("foo", "bar", "baz", "grldsajkopallkjasd");
foreach($input as $i) {
    printf("%u\n", crc32($i));
}
}
