<?hh

class Foo {
	public $prop = array(3);
	function __get($name) {
		return array(4);
	}
}
<<__EntryPoint>>
function main_entry(): void {
  $x = array(1);
  $a = "x";

  $x = array(array(2));
  $x[0];

  $x = "str";
  $x[0];
  $x[3];
  $x = new Foo();
  $x->prop;
  $x->y;
  echo "ok\n";
}
