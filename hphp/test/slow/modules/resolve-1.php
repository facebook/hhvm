<?hh

<<file:__EnableUnstableFeatures("modules")>>
module b;

<<__EntryPoint>>
function main() {
  include "resolve-1.inc1";
  include "resolve-1.inc";

  $f = bar1(); $f();
  $f = bar2(); $f();
  $f = bar3(); $f();
  $f = bar4(); $f();
  $f = bar5(); $f();
  $f = bar6(); $f();

  $f = bar7(); $f();
  $f = bar8(); $f();

  $c = cbar1(); new $c;
  $c = cbar2(); new $c;
  $c = cbar3()(); new $c;
}
