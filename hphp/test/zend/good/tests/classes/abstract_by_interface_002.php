<?hh

class Root {
}

interface MyInterface
{
	static function MyInterfaceFunc():mixed;
}

abstract class Derived extends Root implements MyInterface {
}

class Leaf extends Derived
{
	static function MyInterfaceFunc() :mixed{}
}

<<__EntryPoint>>
function entrypoint_abstract_by_interface_002(): void {

  var_dump(new Leaf);

  require(__DIR__.'/abstract_by_interface_002.inc');

  echo "===DONE===\n";
}
