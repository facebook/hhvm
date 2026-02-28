<?hh

interface I {
  function i1(): int; // legal
  function i2(): int { } // error2041
}

class C {
  function c1(): void; // error2015
  function c2(): void { } // legal
}

trait T {
  function t1() : void; // error2015
  function t2() : void { } // legal
}
