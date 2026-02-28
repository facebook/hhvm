<?hh <<__EntryPoint>> function main(): void {
$array1 = vec[1,
                "hello",
                1,
                "world",
                "hello",
                "",
                "rabbit",
                "foo",
                "Foo",
                TRUE,
                FALSE,
                NULL,
                0];
var_dump(array_count_values($array1));
}
