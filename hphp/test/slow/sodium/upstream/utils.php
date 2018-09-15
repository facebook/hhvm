<?php
$a = 'test';
sodium_memzero($a);
if ($a !== 'test') {
  echo strlen($a);
} else {
  echo $a;
}
echo "\n";
$b = 'string';
$c = 'string';
var_dump(!sodium_memcmp($b, $c));
var_dump(!sodium_memcmp($b, 'String'));
$v = "\xFF\xFF\x80\x01\x02\x03\x04\x05\x06\x07";
$v .= "\x08";
sodium_increment($v);
var_dump(bin2hex($v));

$w = "\x01\x02\x03\x04\x05\x06\x07\x08\xFA\xFB";
$w .= "\xFC";
sodium_add($v, $w);
var_dump(bin2hex($v));

if (SODIUM_LIBRARY_MAJOR_VERSION > 7 ||
	(SODIUM_LIBRARY_MAJOR_VERSION == 7 &&
	 SODIUM_LIBRARY_MINOR_VERSION >= 6)) {
	$v_1 = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";
	$v_2 = ""."\x02\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";
	$v_1 .= '';
	var_dump(sodium_compare($v_1, $v_2));
	sodium_increment($v_1);
	var_dump(sodium_compare($v_1, $v_2));
	sodium_increment($v_1);
	var_dump(sodium_compare($v_1, $v_2));
} else {
	// Dummy test results for libsodium < 1.0.4
	var_dump(-1, 0, 1);
}

$str = 'stdClass';
sodium_memzero($str);
$obj = (object)array('foo' => 'bar');
var_dump($obj);
?>
