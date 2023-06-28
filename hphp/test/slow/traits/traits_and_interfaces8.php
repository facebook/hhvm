<?hh

interface IParent {
  function f():mixed;
}
interface IChild {
  function g():mixed;
}

trait TrParent implements IParent {}

trait TrChild implements IChild {
  use TrParent;
}

function main() :mixed{
  $rc = new ReflectionClass(TrChild::class);
  var_dump($rc->getInterfaceNames());

  foreach ($rc->getMethods() as $meth) {
    echo $meth->class, '::', $meth->name,
      $meth->isAbstract() ? ' (abstract)' : '', "\n";
  }
}


<<__EntryPoint>>
function main_traits_and_interfaces8() :mixed{
main();
}
