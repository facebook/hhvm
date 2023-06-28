<?hh

function f() :mixed{
	$a = darray[];
	$a[0] = $a;
	var_dump($a);
	$b = varray[darray[]];
	$b[0][0] = $b;
	var_dump($b);
}
<<__EntryPoint>>
function main_entry(): void {
  $a = darray[];
  $a[0] = $a;
  var_dump($a);
  $b = varray[darray[]];
  $b[0][0] = $b;
  var_dump($b);
  f();
}
