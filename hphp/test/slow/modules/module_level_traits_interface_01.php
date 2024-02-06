<?hh

module MLT_A;

<<__EntryPoint>>
function main() : void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_module_c.inc';
  include 'module_level_traits_interface_01.inc0';
  include 'module_level_traits_interface_01.inc1';

  (new C())->foo();
}
