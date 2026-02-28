<?hh

<<__EntryPoint>>
function main() {
  include 'basic_visibility.inc0';
  include 'basic_visibility.inc1';
  include 'basic_visibility.inc2';
  (new B())->foobar();
}
