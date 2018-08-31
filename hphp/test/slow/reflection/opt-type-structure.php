<?hh

abstract class Foo {
  abstract const type TCls;

  function hello() {
    echo type_structure(static::class, 'TCls')['classname'];
  }
}

class Bar extends Foo {
  const type TCls = Foo;
}
class Baz extends Foo {
  const type TCls = DOMDocument;
}
class Biz extends Foo {
  const type TCls = Bar;
}
class Buz extends Foo {
  const type TCls = Awaitable;
}

function main() {
  (new Bar)->hello(); echo ", ";
  (new Baz)->hello(); echo ", ";
  (new Biz)->hello(); echo ", ";
  (new Buz)->hello(); echo "\n";
}


<<__EntryPoint>>
function main_opt_type_structure() {
for ($i = 0; $i < 10; $i++) main();
}
