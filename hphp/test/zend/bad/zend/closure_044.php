<?php
/* A non-static closure has a bound instance if it has a scope
 * and doesn't have an instance if it has no scope */

$nonstaticUnscoped = function () { var_dump(isset(A::$priv)); var_dump(isset($this)); };

class A {
	private static $priv = 7;
	function getClosure() {
		return function() { var_dump(isset(A::$priv)); var_dump(isset($this)); };
	}
}

$a = new A();
$nonstaticScoped = $a->getClosure();

echo "Before binding", "\n";
$nonstaticUnscoped(); echo "\n";
$nonstaticScoped(); echo "\n";

echo "After binding, null scope, no instance", "\n";
$d = $nonstaticUnscoped->bindTo(null, null); $d(); echo "\n";
$d = $nonstaticScoped->bindTo(null, null); $d(); echo "\n";

echo "After binding, null scope, with instance", "\n";
$d = $nonstaticUnscoped->bindTo(new A, null); $d(); echo "\n";
$d = $nonstaticScoped->bindTo(new A, null); $d(); echo "\n";

echo "After binding, with scope, no instance", "\n";
$d = $nonstaticUnscoped->bindTo(null, 'A'); $d(); echo "\n";
$d = $nonstaticScoped->bindTo(null, 'A'); $d(); echo "\n";

echo "After binding, with scope, with instance", "\n";
$d = $nonstaticUnscoped->bindTo(new A, 'A'); $d(); echo "\n";
$d = $nonstaticScoped->bindTo(new A, 'A'); $d(); echo "\n";

echo "Done.\n";
