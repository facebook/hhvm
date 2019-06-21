<?hh

namespace foo\bar {
  const baz = 42;
}

namespace {
  use const foo\bar\baz;

  <<__EntryPoint>> function main(): void {
  \var_dump(baz);
  }
}
