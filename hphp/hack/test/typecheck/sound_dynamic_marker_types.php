<?hh

namespace Test {

  function f(\HH\FIXME\POISON_MARKER<string> $poison): void {
    hh_show($poison);
  }
}
