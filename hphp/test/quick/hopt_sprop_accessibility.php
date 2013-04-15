<?php

$nonstaticUnscoped = function () {
  var_dump(A::$priv);
};

class A {
	private static $priv = 7;
	function readVar() {
		A::$priv;
	}
}

(new A())->readVar();
$nonstaticUnscoped();
