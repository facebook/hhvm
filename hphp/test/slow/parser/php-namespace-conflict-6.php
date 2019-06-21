<?hh
namespace Blah {
  function foo(string $x) {}
  use Baz\string;
  function bar(string $x) {}
  <<__EntryPoint>> function main(): void {
  echo "Done\n";
  }
}
