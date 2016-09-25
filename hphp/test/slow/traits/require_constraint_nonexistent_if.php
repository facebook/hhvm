<?hh

trait T1 {
  require implements NonExistent;
}

class X {
  use T1;
};
