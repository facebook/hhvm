////foo.php
<?hh

trait Tfoo {
  public function bar() : int {
    return 6;
  }
}

////foo_impl.php
<?hh

interface Ifoo {
  public function bar() : void;
}

class Foo implements Ifoo{
  use Tfoo;
}
