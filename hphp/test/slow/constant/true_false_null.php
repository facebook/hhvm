<?hh

namespace Foo {
  const NOT_REALLY_TRUE = false;
  const ANTI_NULL = true;
  const FAKE_FALSE = 1;
}

namespace {
  use const Foo\NOT_REALLY_TRUE as true;
  use const Foo\FAKE_FALSE as false;
  use const Foo\ANTI_NULL as null;

  <<__EntryPoint>>
  function main() :mixed{
    var_dump(true);
    var_dump(false);
    var_dump(null);
  }
}
