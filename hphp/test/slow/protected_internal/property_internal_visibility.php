<?hh

<<__EntryPoint>>
function main() {
  include 'property_internal_visibility.inc0';
  include 'property_internal_visibility.inc1';
  include 'property_internal_visibility.inc2';
  include 'property_internal_visibility.inc3';
  (new B())->foobar();
}
