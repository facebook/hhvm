<?hh
namespace test\ns1;

function foo() {
  echo __FUNCTION__,"\n";
}

function bar() {
  echo __FUNCTION__,"\n";
}

<<__EntryPoint>> function main(): void {
foo();
\test\ns1\foo();
bar();
\test\ns1\bar();
}
