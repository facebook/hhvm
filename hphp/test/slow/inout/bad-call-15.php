<?hh

function foo(inout $a) {}

class Herp {
  function derp() {
    foo(inout $this[12]);
  }
}

(new Herp)->derp();
