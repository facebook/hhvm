<?hh

function parse_callback(...$args) {
  var_dump($args);
}
function main() {
  $p = xml_parser_create();
  xml_set_element_handler($p, 'parse_callback', 'parse_callback');
  xml_set_element_handler($p, false, 'parse_callback');
  xml_parse($p, "<tag><child/></tag>", true);

  $p = xml_parser_create();
  xml_set_element_handler($p, 'parse_callback', 'parse_callback');
  xml_set_element_handler($p, 'parse_callback', '');
  xml_parse($p, "<tag><child/></tag>", true);
}

<<__EntryPoint>>
function main_1668() {
main();
}
