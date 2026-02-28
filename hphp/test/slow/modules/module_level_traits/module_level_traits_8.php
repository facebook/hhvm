<?hh

module MLT_B;

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_module_c.inc';
  include 'module_level_traits_8.inc0';
  include 'module_level_traits_8.inc1';

  (new C())->foo();
}
