<?hh

namespace HH\Lib\Dict\Dict {

  function f(): void {
    echo __FUNCTION__."\n";
  }

}

namespace HH\Lib\Dict {

  function f(): void {
    echo __FUNCTION__."\n";
  }

  <<__EntryPoint>> function main(): void {
  Dict\f(); // HH\Lib\Dict\f
  }
}
