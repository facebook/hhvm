<?hh

module MLT_C;

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_module_c.inc';
  include 'module_level_traits_4.inc0';
  include 'module_level_traits_4.inc1';
  include 'module_level_traits_4.inc2';

  (new C())->getFoo();  // should work...
}
