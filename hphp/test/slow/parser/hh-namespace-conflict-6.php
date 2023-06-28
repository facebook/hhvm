<?hh
namespace Blah {
  function foo(string $x) :mixed{}
  use Baz\string;
  function bar(string $x) :mixed{}
  <<__EntryPoint>> function main(): void {
  echo "Done\n";
  }
}
