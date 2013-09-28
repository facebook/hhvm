<?php
$tests = array(
    "\x8F\xA1\xFF", //2 sub as 2nd is potentially valid as lead
    "\x8F\xA1", //2 sub, as 2nd is potentially valid as lead
    "\x8F", //1 sub
    "\x8F\xA0", //1 sub, A0 is not valid as sole/first byte
    "\x8F\xA1\x21", //2 sub, no consume last
    "\x8F\x21", //1 sub, no consume last
    "\x8E\xAE", //valid
    "\x8E", //1 sub
    "\x8E\x21", //1 sub, no consume last
    "\xB2\xFF", //1 sub
    "\xB2", //1 sub
    "\xB2\x21", //1 sub, no consume last
    "\xA0", //1 sub
);

foreach ($tests as $test) {
    error_reporting(~E_STRICT);
    $a = htmlentities($test, ENT_QUOTES | ENT_SUBSTITUTE, "EUC-JP");
    error_reporting(-1);
	var_dump($a, bin2hex($a));
    $a = htmlspecialchars($test, ENT_QUOTES | ENT_SUBSTITUTE, "EUC-JP");
	var_dump($a, bin2hex($a));
    echo "\n";
}
?>