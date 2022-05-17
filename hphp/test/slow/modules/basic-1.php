<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

<<__EntryPoint>>
function main() {
  include 'basic-1.inc';
  Cls::foo_static();
  (new Cls)->foo();
}
