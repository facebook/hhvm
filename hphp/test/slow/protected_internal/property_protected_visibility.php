<?hh

<<__EntryPoint>>
function main() {
  include 'property_protected_visibility.inc0';
  include 'property_protected_visibility.inc1';
  include 'property_protected_visibility.inc2';
  (new B())->foobar();
}
