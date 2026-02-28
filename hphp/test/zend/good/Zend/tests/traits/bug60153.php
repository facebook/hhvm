<?hh

interface IFoo {
    public function oneArgument($a):mixed;
}

trait TFoo {
  public function oneArgument() :mixed{}
}

class C implements IFoo {
  use TFoo;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
