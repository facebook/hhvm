<?hh // partial

function consumeArrayOfIntToInt(array<int, int> $_): void {}

function test(array<int> $arrayOfInt): void {
  consumeArrayOfIntToInt($arrayOfInt);
}
