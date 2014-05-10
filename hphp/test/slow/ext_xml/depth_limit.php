<?php

function main() {
  $a = array();
  $res = xml_parse_into_struct(
    xml_parser_create_ns(),
    str_repeat("<blah>", 100000),
    $a);
  var_dump(count(array_keys($a)));
}
main();
