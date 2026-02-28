<?hh
namespace test\ns1;

function foo() :mixed{
  echo __FUNCTION__,"\n";
}

function bar() :mixed{
  echo __FUNCTION__,"\n";
}

<<__EntryPoint>> function main(): void {
foo();
\test\ns1\foo();
bar();
\test\ns1\bar();
}
