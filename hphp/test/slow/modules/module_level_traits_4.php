<?hh

module MLT_C;

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_modules.inc';
  include 'module_level_traits_4.inc0';
  include 'module_level_traits_4.inc1';
  include 'module_level_traits_4.inc2';

  (new C())->getFoo();  // should work...
}
