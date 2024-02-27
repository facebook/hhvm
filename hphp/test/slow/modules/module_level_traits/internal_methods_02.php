<?hh

module MLT_A;

<<__EntryPoint>>
function main(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'internal_methods_02.inc0';
  include 'internal_methods_02.inc1';
  include 'internal_methods_02.inc2';

  (new C())->foo();
}
