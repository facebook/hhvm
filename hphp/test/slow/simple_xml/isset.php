<?hh
function test_isset($str) :mixed{
  $xml = '<field Default="' . $str . '"/>';
  $field = new SimpleXMLElement($xml);
  var_dump(isset($field));
  var_dump($field->offsetExists('Default') && $field->offsetGet('Default') !== null);
  var_dump($field->offsetExists('XXX') && $field->offsetGet('XXX') !== null);
}


<<__EntryPoint>>
function main_isset() :mixed{
test_isset("");
test_isset("No");
test_isset("Yes");
test_isset("NULL");
test_isset("0");
test_isset("1");
test_isset("20");
}
