<?hh

trait T {
  require extends A;
  require implements A;
};

class X {
  use T;
}
