<?hh

function f() {
	throw new Exception();
}

function g($x) {
	return "'".(string)$x[0]."'";
}
<<__EntryPoint>>
function main_entry(): void {

  $count = -1;
  try {
    var_dump(preg_replace_callback('/\w/', fun('f'), 'z', -1, inout $count));
  } catch(Exception $e) {}

  var_dump(preg_replace_callback('@\b\w{1,2}\b@', fun('g'), array('a b3 bcd', 'v' => 'aksfjk', 12 => 'aa bb'), -1, inout $count));

  var_dump(preg_replace_callback('~\A.~', fun('g'), array(array('xyz')), -1, inout $count));

  var_dump(preg_replace_callback('~\A.~', $m ==> strtolower($m[0]), 'ABC', -1, inout $count));
}
