<?hh

namespace Foo {
  const string foo_a = 'foo_const_a';
  const string foo_b = 'foo_const_b';

  namespace Bar {
    const string bar_a = 'foo_bar_const_a';
    const string bar_b = 'foo_bar_const_b';
  }
}

function test(): void { Foo\fooAUTO332
