<?hh
namespace Blah {
  use Blah\string;
  function foo(string $x) :mixed{}
  class string {}
  function bar(string $x) :mixed{}
  <<__EntryPoint>> function main(): void {
  echo "Done\n";
  }
}
