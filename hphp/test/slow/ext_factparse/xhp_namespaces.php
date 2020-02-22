<?hh // strict

namespace {
  class :foo:bar {}
}

namespace HerpDerp\SubNS {
  class MyClass {}
}

namespace HerpDerp {
  class MyClass {}

  xhp class ExtendsFooBar extends :foo:bar {}
  xhp class ExtendsHerpDerpMyClass extends MyClass {}
  xhp class ExtendsHerpDerpSubNSMyClass extends SubNS\MyClass {}
}

namespace {
  <<__EntryPoint>>
  function main(): void {
    foreach (HH\facts_parse('/', varray[__FILE__], false, true)[__FILE__]['types'] as $type) {
      \var_dump(dict[
        'name' => $type['name'],
        'extends' => $type['baseTypes'][0] ?? null,
      ]);
    }
  }
}
