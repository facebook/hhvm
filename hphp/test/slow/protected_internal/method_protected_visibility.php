<?hh

<<__EntryPoint>>
function main() {
  include 'method_protected_visibility.inc0';
  include 'method_protected_visibility.inc1';
  include 'method_protected_visibility.inc2';
  (new B())->foobar();
}
