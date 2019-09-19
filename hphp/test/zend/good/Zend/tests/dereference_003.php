<?hh

class  foo {
	public $x = 2;
	public function a() {
		$x = array();
		$x[] = new foo;
		return $x;
	}
	public function b() {
		return array(1.2, array(new self));
	}
	public function c(&$a, &$b) {
		$a = array();
		$b[] = true;
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
  var_dump($foo->c(&$a, &$a)[0]);
  var_dump($foo->d()[0]);
}
