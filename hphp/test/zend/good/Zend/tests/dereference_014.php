<?hh

class foo {
	public $x;
	static public $y;
		
	public function a() :mixed{
		return $this->x;
	}
	
	static public function b() :mixed{
		return self::$y;
	}
}

<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);

  $foo = new foo;
  $h = $foo->a()[0]->a;
  var_dump($h);

  $h = foo::b()[1]->b;
  var_dump($h);
}
