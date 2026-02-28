<?hh

class foo {
  const bar = "fubar\n";

  function __construct($arg = self::bar) {
    echo $arg;
  }
}

<<__EntryPoint>>
function main() :mixed{
  new foo();
}
