<?hh
namespace Foo {
  class Derived extends Bar\Base {
    const NUM = '42';
    public function set($k = vec[\SORT_NUMERIC, vec[self::NUM], parent::STR]) :mixed{
      \var_dump($k);
    }
  }
}

namespace Foo\Bar {
  class Base {
    const STR = 'foo';
  }
}

namespace {
  <<__EntryPoint>> function main(): void {
    echo "reflection:\n";
    $rc = (new \ReflectionClass("Foo\\Derived"))->getMethod('set');
    \var_dump($rc->getParameters()[0]->getDefaultValue());
    \var_dump($rc->getParameters()[0]->getDefaultValueText());
    \var_dump($rc->getParameters()[0]->getDefaultValueConstantName());
    echo "call:\n";
    (new Foo\Derived())->set();
  }
}
