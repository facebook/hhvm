<?hh

function main() :mixed{
  $a = vec[];
  $b = vec[];
  $res = xml_parse_into_struct(
    xml_parser_create_ns(),
    str_repeat("<blah>", 100000),
    inout $a,
    inout $b
  );
  var_dump(count(array_keys($a)));
}

<<__EntryPoint>>
function main_depth_limit() :mixed{
main();
}
