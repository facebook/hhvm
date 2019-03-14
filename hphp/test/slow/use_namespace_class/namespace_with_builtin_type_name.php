<?hh

namespace Foo\Dict {
  function herp(dict<int, string> $derp): void {
    \var_dump($derp);
  }
}

namespace {
  use Foo\Dict;

  function main(): void {
    Dict\herp(dict[1 => 'foo', 42 => 'bar']);
  }

  main();
}
