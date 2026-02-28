<?hh

class Root {
}

interface MyInterface
{
	function MyInterfaceFunc():mixed;
}

abstract class Derived extends Root implements MyInterface {
}

class Leaf extends Derived
{
	function MyInterfaceFunc() :mixed{}
}

<<__EntryPoint>>
function entrypoint_abstract_by_interface_001(): void {

  var_dump(new Leaf);

  require(__DIR__.'/abstract_by_interface_001.inc');

  echo "===DONE===\n";
}
