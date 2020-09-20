<?hh // partial

function consumeArrayOfIntToInt(array<int, int> $_): void {}

function test(int ...$variadicOfInt): void {
  consumeArrayOfIntToInt($variadicOfInt);
}
