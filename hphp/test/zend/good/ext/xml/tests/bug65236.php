<?hh <<__EntryPoint>> function main(): void {
$a = varray[];
$b = varray[];
xml_parse_into_struct(xml_parser_create_ns(), str_repeat("<blah>", 1000), inout $a, inout $b);

echo "Done\n";
}
