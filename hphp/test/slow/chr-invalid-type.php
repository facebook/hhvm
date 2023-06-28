<?hh


<<__EntryPoint>>
function main_chr_invalid_type() :mixed{
$inputs = varray[
  -1, 0, 1, 2, 128, 255,
  "0string", "1string", "2",
  varray[], varray[1], varray[1,2],
  new stdClass,
  new SimpleXMLElement("<foo />"),
  new SimpleXMLElement("<foo><bar/></foo>"),
  fopen(__FILE__, 'r'),
];

foreach ($inputs as $v) {
  var_dump(bin2hex(chr($v)));
}
}
