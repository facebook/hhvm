<?hh

<<__EntryPoint>>
function main() {
  include 'method_protected_internal_visibility.inc0';
  include 'method_protected_internal_visibility.inc1';
  include 'method_protected_internal_visibility.inc2';
  include 'method_protected_internal_visibility.inc3';
  (new B())->foobar();
}
