<?hh
interface IFoo {
    public function blah():mixed;
}

trait T implements IFoo {}

class C {
  use T;
}

function main() :mixed{
  $c = new C();
}


<<__EntryPoint>>
function main_traits_and_interfaces3() :mixed{
main();
}
