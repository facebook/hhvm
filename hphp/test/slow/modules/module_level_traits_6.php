<?hh

module MLT_B;

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_modules.inc';
  include 'module_level_traits_6.inc0';
  include 'module_level_traits_6.inc1';

  (new C())->getFoo();  // should work...
}
