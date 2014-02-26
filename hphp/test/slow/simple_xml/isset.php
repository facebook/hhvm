<?php
function test_isset($str) {
  $xml = '<field Default="' . $str . '"/>';
  $field = new SimpleXmlElement($xml);
  var_dump(isset($field));
  var_dump(isset($field['Default']));
  var_dump(isset($field['XXX']));
}

test_isset("");
test_isset("No");
test_isset("Yes");
test_isset("NULL");
test_isset("0");
test_isset("1");
test_isset("20");
