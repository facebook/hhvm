<?hh

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_module_c.inc';
  include 'internal_01.inc0';
  include 'internal_01.inc1';

  (new C())->foo();
}
