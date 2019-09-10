<?hh

function f() {
	throw new Exception();
}

$count = -1;
try {
var_dump(preg_replace_callback('/\w/', 'f', 'z', -1, inout $count));
} catch(Exception $e) {}

function g($x) {
	return "'".(string)$x[0]."'";
}

var_dump(preg_replace_callback('@\b\w{1,2}\b@', 'g', array('a b3 bcd', 'v' => 'aksfjk', 12 => 'aa bb'), -1, inout $count));

var_dump(preg_replace_callback('~\A.~', 'g', array(array('xyz')), -1, inout $count));

var_dump(preg_replace_callback('~\A.~', $m ==> strtolower($m[0]), 'ABC', -1, inout $count));
