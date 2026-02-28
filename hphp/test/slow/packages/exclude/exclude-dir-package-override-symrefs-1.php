<?hh

// 1. function access_test_package_override_2() is in package intern
//    package intern is not part of the active deployment but file has
//    a package override
// 2. function access_test_symbolrefs_2() is in an intern file that has no
//    package override, must be pulled in by SymbolRefs
// 3. intern is excluded via --exclude-dir

// The call to access_test_package_override_2 must succeed because
// --exclude-pattern cannot be overridden

<<__EntryPoint>>
function exclude_dir_package_override_symrefs_1(): void {
  base();
  $f = access_test_package_override_2<>;
  $f();                       // should succeed because of package override
  access_test_symbolrefs_2(); // should succeed because of symbol refs
}
