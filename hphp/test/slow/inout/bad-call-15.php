<?hh

function foo(inout $a) {}

class Herp {
  function derp() {
    foo(inout $this[12]);
  }
}


<<__EntryPoint>>
function main_bad_call_15() {
(new Herp)->derp();
}
