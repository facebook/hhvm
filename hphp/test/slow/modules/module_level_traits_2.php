<?hh

module MLT_B;

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_modules.inc';
  include 'module_level_traits_2.inc0';
  include 'module_level_traits_2.inc1';
  include 'module_level_traits_2.inc2';

  (new C())->getFoo();  // should work...
}
