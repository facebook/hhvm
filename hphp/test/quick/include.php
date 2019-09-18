<?hh

function foo() {
  require 'include.2.inc';
}
<<__EntryPoint>>
function main_entry(): void {

  $a = 1;#"a\n";
  print $a."\n";

  require 'include.1.inc';

  print $a."\n";
  print $b."\n";
  foo();

  $path = dirname(__FILE__) . '/include.3.inc';
  require $path;

  $path = __DIR__ . '/include.3.inc';
  require $path;

  if (__hhvm_intrinsics\launder_value(0)) {
    // to ensure we include the file in
    // RepoAuthoritative mode
    require 'include.3.inc';
  }
}
