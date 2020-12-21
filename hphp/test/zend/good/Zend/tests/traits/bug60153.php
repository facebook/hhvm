<?hh

interface IFoo {
    public function oneArgument($a);
}

trait TFoo {
  public function oneArgument() {}
}

class C implements IFoo {
  use TFoo;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
