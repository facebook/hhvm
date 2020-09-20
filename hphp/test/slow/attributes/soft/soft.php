<?hh

async function f1(<<__Soft>> int $_): Awaitable<<<__Soft>> int> {
  return 'hello';
}

async function f2(<<__Soft()>> int $_): <<__Soft>> Awaitable<int> {
  return 'goodbye';
}

function g(): void {
  f1(3.14);
  f2(2.72);
}

class C {
  <<__Soft>> private int $x = 'test';
}

<<__EntryPoint>>
function main(): void {
  g();
  new C();
}
