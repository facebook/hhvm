<?php
for ($i = 65; $i < 256; $i++) {
	if ($i >= 0xc0) {
		$v = chr(0xc3) . chr($i - 64);
	} elseif ($i >= 0x80) {
		$v = chr(0xc2) . chr($i);
	} else {
		$v = chr($i); // make it UTF-8
	}
	$ret = wddx_serialize_value($v);
	echo $ret . "\n";
	var_dump(bin2hex($v), bin2hex(wddx_deserialize($ret)), $v == wddx_deserialize($ret));
}
?>
