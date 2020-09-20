<?hh

class  foo {
	public $x = 2;
	public function a() {
		$x = varray[];
		$x[] = new foo;
		return $x;
	}
	public function b() {
		return varray[1.2, varray[new self]];
	}
	public function c(inout $a, inout $b) {
		$a = varray[];
		$a[] = true;
		return $a;
	}
	public function d() {
		return $this->b();
	}
}

<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);

  $foo = new foo;

  var_dump($foo->a()[0]->x);
  var_dump($foo->a()[0]);
  var_dump($foo->b()[1][0]->a()[0]->x);
  $a = null;
  var_dump($foo->c(inout $a, inout $a)[0]);
  var_dump($foo->d()[0]);
}
