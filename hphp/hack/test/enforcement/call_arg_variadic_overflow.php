<?hh

function takes_variadic_overflow(int $first, int ...$rest): void {}

function test(): void {
  takes_variadic_overflow(1, 2, 3);
//                              ^ enforcement-at-caret
}
