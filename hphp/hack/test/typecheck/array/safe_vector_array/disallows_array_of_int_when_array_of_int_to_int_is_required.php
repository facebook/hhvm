<?hh

function consumeArrayOfIntToInt(darray<int, int> $_): void {}

function test(varray<int> $arrayOfInt): void {
  consumeArrayOfIntToInt($arrayOfInt);
}
