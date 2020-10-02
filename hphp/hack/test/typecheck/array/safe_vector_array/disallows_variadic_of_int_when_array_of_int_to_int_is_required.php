<?hh // partial

function consumeArrayOfIntToInt(darray<int, int> $_): void {}

function test(int ...$variadicOfInt): void {
  consumeArrayOfIntToInt($variadicOfInt);
}
