<?hh

class Object {
	public function __construct() {
	}
}

class Object1 {
	public function __construct($var) {
		var_dump($var);
	}
}
<<__EntryPoint>>
function main_entry(): void {

  $class= new ReflectionClass('Object');
  var_dump($class->newInstanceArgs());

  $class= new ReflectionClass('Object1');
  try { var_dump($class->newInstanceArgs()); } catch (Exception $e) { var_dump($e->getMessage()); }
  var_dump($class->newInstanceArgs(varray['test']));


  echo "Done\n";
}
