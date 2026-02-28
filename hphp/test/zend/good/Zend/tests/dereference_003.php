<?hh

class  foo {
	public $x = 2;
	public function a() :mixed{
		$x = vec[];
		$x[] = new foo;
		return $x;
	}
	public function b() :mixed{
		return vec[1.2, vec[new self]];
	}
	public function c(inout $a, inout $b) :mixed{
		$a = vec[];
		$a[] = true;
		return $a;
	}
	public function d() :mixed{
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
