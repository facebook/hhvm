<?php
namespace X;
use X as Y;
class X {
	const C = "const ok\n";
	static $var = "var ok\n";
	function __construct() {
		echo "class ok\n";
	}
	static function bar() {
		echo "method ok\n";
	}
}
new X();
new Y\X();
new \X\X();
X::bar();
Y\X::bar();
\X\X::bar();
echo X::C;
echo Y\X::C;
echo \X\X::C;
echo X::$var;
echo Y\X::$var;
echo \X\X::$var;