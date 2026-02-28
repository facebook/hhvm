<?hh

type T = int;

function test(): void {
  hh_expect<string>(nameof T);
}
