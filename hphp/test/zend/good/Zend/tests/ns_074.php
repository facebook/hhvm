<?hh

namespace {
  class bar {}
}

namespace foo {
  class bar extends \bar {}

  <<__EntryPoint>>
  function main() :mixed{
    $x = function (?\bar $x = NULL) {
      \var_dump($x);
    };

    $x(NULL);
    $x(new bar());
    $x(new \bar());
  }
}
