<?hh
interface IFoo {
    public function blah();
}

trait T implements IFoo {}

class C {
  use T;
}

function main() {
  $c = new C();
}


<<__EntryPoint>>
function main_traits_and_interfaces3() {
main();
}
