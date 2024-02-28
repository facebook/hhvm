<?hh
// RUN: %hackc -vHack.Lang.OptimizeIsTypeChecks=true --test-compile-with-decls %s | FileCheck %s
// RUN: %hackc -vHack.Lang.OptimizeIsTypeChecks=true --test-compile-with-decls --use-serialized-decls %s | FileCheck %s

function test() {
  $int is int;
  // CHECK: CGetL $int
  // CHECK: IsTypeC Int

  $bool as bool;
  // CHECK: CGetL $bool
  // CHECK: IsTypeC Bool

  $vec is vec<_>;
  // CHECK: CGetL $vec
  // CHECK: IsTypeC Vec

  $ks as keyset<_>;
  // CHECK: CGetL $ks
  // CHECK: IsTypeC Keyset

  $dict is dict<_, _>;
  // CHECK: CGetL $dict
  // CHECK: IsTypeC Dict

  $foo is Bar;
  // CHECK: CGetL $foo
  // CHECK: IsTypeStructC Resolve Deep

  $hello as World;
  // CHECK: CGetL $hello
  // CHECK: IsTypeStructC Resolve Deep

  $str is string;
  // CHECK: CGetL $str
  // CHECK: IsTypeC Str
}

