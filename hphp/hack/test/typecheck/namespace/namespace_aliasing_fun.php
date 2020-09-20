<?hh // partial

namespace HH\Lib\Dict {
  function foo(): int {
    return 1;
  }
}

namespace {
  function main() {
    $f = fun('Dict\foo');
    expect_int($f());

    $g = fun('HH\\Lib\\Dict\\foo');
    expect_int($g());

    $h = fun('\\HH\\Lib\\Dict\\foo');
    expect_int($h());

    $i = fun('HH\Lib\Dict\foo');
    expect_int($i());

    $j = fun('\HH\Lib\Dict\foo');
    expect_int($j());
  }

  function expect_int(int $x): void {}
}
