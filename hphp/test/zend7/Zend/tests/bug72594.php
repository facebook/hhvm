<?php
if (isset($runtime)) {
	return new class {
		public $bar;
		public function bing($foo = null) {
			if ($foo) $foo->bing();
		}
	};
}

$runtime = 1;
$oldFoo = require(__FILE__);
$newFoo = require(__FILE__);

var_dump(get_class_methods($oldFoo));
var_dump(get_object_vars($oldFoo));

$newFoo->bing($oldFoo);
?>
