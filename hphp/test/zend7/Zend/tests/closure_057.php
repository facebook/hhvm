<?php
class A {
}

function getfunc() {
	$b = function() {
		$a = function() {
		};
		$a();
	};
	return $b->bindTo(new A());
}

call_user_func(getfunc());

echo "okey";
?>
