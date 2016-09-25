<?hh

trait T1 {
  require extends NonExistent;
}

class X {
  use T1;
};
