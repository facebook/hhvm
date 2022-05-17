<?hh

<<__EntryPoint>>
function main() {
  include 'basic-2.inc';
  Cls::foo_static();
  (new Cls)->foo();
}
