<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

<<__EntryPoint>>
function main() {
  include 'basic-3.inc';
  new InternalCls();
  new ReifiedInternalCls();
}
