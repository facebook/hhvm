<?hh

// without module level traits, this would not raise an error in repo mode
// because trait T gets inlined inside module B, where foo is also defined

module MLT_B;

internal function foo(): void { echo "foo\n"; }

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_module_c.inc';
  include 'module_level_traits_3.inc0';
  include 'module_level_traits_3.inc1';

  (new C())->getFoo();
}
