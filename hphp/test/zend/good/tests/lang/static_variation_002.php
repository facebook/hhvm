<?php

Class C {
	function f() {
		static $a = array(1,2,3);
		eval(' static $k = array(4,5,6); ');
		
		function cfg() {
			static $a = array(7,8,9);
			eval(' static $k = array(10,11,12); ');
			var_dump($a, $k);
		}
		var_dump($a, $k);
	}
}
$c = new C;
$c->f();
cfg();

Class D {
	static function f() {
		eval('function dfg() { static $b = array(1,2,3); var_dump($b); } ');
	}
}
D::f();
dfg();

eval(' Class E { function f() { static $c = array(1,2,3); var_dump($c); } }');
$e = new E;
$e->f();

?>