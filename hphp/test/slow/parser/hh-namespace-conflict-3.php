<?hh
namespace Blah {
  use Blah\string;
  function foo(string $x) {}
  class string {}
  function bar(string $x) {}
  <<__EntryPoint>> function main(): void {
  echo "Done\n";
  }
}
