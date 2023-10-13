<?hh

class C {}

trait T1 {
  protected C $c; // responsibility of using class

  abstract protected function __construct();
}

trait T2 {
  private C $c; // responsibility of using class, see chown_privates

  abstract protected function __construct();
}
