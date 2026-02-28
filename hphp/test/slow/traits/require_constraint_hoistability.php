<?hh

interface IA {}
interface IB extends IA {}

trait TB {
  require implements IB;
}

class C implements IB {
  use TB;
}


<<__EntryPoint>>
function main_require_constraint_hoistability() :mixed{
echo 'Done', "\n";
}
