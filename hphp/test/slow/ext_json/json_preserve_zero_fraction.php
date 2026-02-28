<?hh

<<__EntryPoint>>
function main_json_preserve_zero_fraction() :mixed{
  $o = new stdClass;
  $o->float = 12.0;
  $o->integer = 12;
echo "* Testing JSON output\n\n";
var_dump(json_encode(12.3, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(12, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(12.0, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(0.0, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(vec[12, 12.0, 12.3], JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode($o, JSON_PRESERVE_ZERO_FRACTION));

echo "\n* Testing encode/decode symmetry\n\n";

var_dump(json_decode(json_encode(12.3, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(12, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(12.0, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(0.0, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(vec[12, 12.0, 12.3], JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode($o, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode($o, JSON_PRESERVE_ZERO_FRACTION), true));
}
