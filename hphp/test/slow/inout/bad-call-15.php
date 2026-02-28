<?hh

function foo(inout $a) :mixed{}

class Herp {
  function derp() :mixed{
    foo(inout $this[12]);
  }
}


<<__EntryPoint>>
function main_bad_call_15() :mixed{
(new Herp)->derp();
}
