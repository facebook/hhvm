<?php

$filename = dirname(__FILE__)."/fgetss.html";

$array = array(
	"askasdfasdf<b>aaaaaa\n</b>dddddd",
	"asdqw<i onClick=\"hello();\">\naaaa<>qqqq",
	"aaa<script>function foo() {}</script>qqq",
	"asdasd<a\n asdjeje",
	"",
	"some text \n<b>blah</i>",
	"some another text <> hoho </>"
	);

foreach ($array as $str) {
	file_put_contents($filename, $str);
	$fp = fopen($filename, "r");
	var_dump(fgetss($fp));
	var_dump(fgetss($fp));
}

foreach ($array as $str) {
	file_put_contents($filename, $str);
	$fp = fopen($filename, "r");
	var_dump(fgetss($fp, 10));
	var_dump(fgetss($fp, 10));
}

var_dump(fgetss($fp, -10));
var_dump(fgetss($fp, 0));
fclose($fp);
var_dump(fgetss($fp, 0));

@unlink($filename);

echo "Done\n";
?>