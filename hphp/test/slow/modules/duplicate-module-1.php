<?hh

<<file:__EnableUnstableFeatures("modules")>>

new module A {}

<<__EntryPoint>>
function main() {
  include 'duplicate-module-1.inc';
}
