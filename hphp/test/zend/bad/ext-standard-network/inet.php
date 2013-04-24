<?php

$packed = chr(127) . chr(0) . chr(0) . chr(1);
var_dump(inet_ntop((binary)$packed));

$packed = chr(255) . chr(255) . chr(255) . chr(0);
var_dump(inet_ntop((binary)$packed));

var_dump(inet_ntop());
var_dump(inet_ntop(-1));
var_dump(inet_ntop(b""));
var_dump(inet_ntop(b"blah-blah"));

var_dump(inet_pton());
var_dump(inet_pton(b""));
var_dump(inet_pton(-1));
var_dump(inet_pton(b"abra"));

$array = array(
	b"127.0.0.1",
	b"66.163.161.116",
	b"255.255.255.255",
	b"0.0.0.0",
	);
foreach ($array as $val) {
	var_dump(bin2hex($packed = inet_pton($val)));
	var_dump(inet_ntop($packed));
}

echo "Done\n";
?>