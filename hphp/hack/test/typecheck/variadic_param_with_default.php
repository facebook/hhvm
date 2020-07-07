<?hh

interface IFoo {

}

final class Foos {
  private vec<IFoo> $foos;
  public function __construct(IFoo ...$foos = vec[]) {
    $this->foos = vec($foos);
  }
}

final class Bar {
  private ?Foos $foos = null;
}
