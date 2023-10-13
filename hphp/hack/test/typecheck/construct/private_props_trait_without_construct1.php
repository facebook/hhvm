<?hh

class C {}

trait T {
  private C $c; // responsibility of using class; see chown_privates
}
