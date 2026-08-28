<?hh
// SKIP_TAGSTART offset (100000) is far past the tag length.
<<__EntryPoint>>
function main(): mixed {
  $p = xml_parser_create();
  xml_parser_set_option($p, XML_OPTION_SKIP_TAGSTART, 100000);
  $vals = null;
  $idx = null;
  xml_parse_into_struct($p, "<a><b/>TEXT</a>", inout $vals, inout $idx);
  echo "parsed " . count($vals) . " values\n";
  echo "ok\n";
}
