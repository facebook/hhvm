<?hh

namespace foo\bar {
  const baz = 42;
}

namespace other {
  const baz = 100;
}

namespace {
  use const foo\bar\baz;
  use const other\baz;

  <<__EntryPoint>> function main(): void {
    \var_dump(baz);
  }
}
