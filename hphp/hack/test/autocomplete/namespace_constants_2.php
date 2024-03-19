<?hh

namespace Foo {
  const string foo_a = 'foo_const_foo_a';
  const string foo_b = 'foo_const_foo_b';
  const string bar_a = 'foo_const_bar_a';

  namespace Bar {
    const string bar_a = 'foo_bar_const_bar_a';
    const string bar_b = 'foo_bar_const_bar_b';
    const string foo_a = 'foo_bar_const_foo_b';
  }
}

function test(): void { Foo\Bar\barAUTO332
