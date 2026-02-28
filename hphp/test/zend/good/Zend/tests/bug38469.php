<?hh

function f() :mixed{
	$a = dict[];
	$a[0] = $a;
	var_dump($a);
	$b = vec[dict[]];
	$b[0][0] = $b;
	var_dump($b);
}
<<__EntryPoint>>
function main_entry(): void {
  $a = dict[];
  $a[0] = $a;
  var_dump($a);
  $b = vec[dict[]];
  $b[0][0] = $b;
  var_dump($b);
  f();
}
