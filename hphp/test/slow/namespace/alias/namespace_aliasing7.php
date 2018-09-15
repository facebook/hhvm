<?hh // strict

namespace HH\Lib\Dict\Dict {

  function f(): void {
    echo __FUNCTION__."\n";
  }

}

namespace HH\Lib\Dict {

  function f(): void {
    echo __FUNCTION__."\n";
  }

  Dict\f(); // HH\Lib\Dict\f

}
