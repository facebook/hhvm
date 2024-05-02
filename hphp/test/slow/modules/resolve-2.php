<?hh


module b;

<<__EntryPoint>>
function main() {
  include "resolve-1.inc1";
  include "resolve-2.inc";

  $f = bar1(); $f();
  $f = bar2(); $f();
  $f = bar3(); $f();
  $f = bar4(); $f();
  $f = bar5(); $f();
  $f = bar6(); $f();
}
