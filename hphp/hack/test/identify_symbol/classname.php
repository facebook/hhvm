<?hh

class C {}

function test(): void {
  hh_expect<string>(nameof C);
}
