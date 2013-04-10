<?php

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	protected $d = "Original d";
	private $e = "Original e";
	
}

echo "\n\nSimple loop.\n";
$obj = new C;
foreach ($obj as $v) {
	var_dump($v);
}
foreach ($obj as $k => $v) {
	var_dump($k, $v);
}
echo "\nCheck key and value after the loop.\n";
var_dump($k, $v);


echo "\n\nObject instantiated inside loop.\n";
foreach (new C as $v) {
	var_dump($v);
}
foreach (new C as $k => $v) {
	var_dump($k, $v);
}
echo "\nCheck key and value after the loop.\n";
var_dump($k, $v);
?>