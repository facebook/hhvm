<?hh

class Foo {
	public $prop = varray[3];
	function __get($name) {
		return varray[4];
	}
}
<<__EntryPoint>>
function main_entry(): void {
  $x = varray[1];
  $a = "x";

  $x = varray[varray[2]];
  $x[0];

  $x = "str";
  $x[0];
  $x[3];
  $x = new Foo();
  $x->prop;
  $x->y;
  echo "ok\n";
}
