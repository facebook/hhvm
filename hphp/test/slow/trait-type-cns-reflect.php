<?hh

trait Tp {
  const type Ta = bool;
}

trait Tc {
  use Tp;

  const type Tb = int;
}

class C {
  use Tc;
}

<<__EntryPoint>>
function main() {
  var_dump((new ReflectionTypeConstant('C', 'Ta'))->getAssignedTypeText());
  var_dump((new ReflectionTypeConstant('C', 'Tb'))->getAssignedTypeText());
}
