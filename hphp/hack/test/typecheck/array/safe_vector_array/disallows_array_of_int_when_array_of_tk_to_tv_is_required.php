<?hh // partial

function consumeArrayOfTkToTv<Tk, Tv>(darray<Tk, Tv> $_): void {}

function test(varray<int> $arrayOfInt): void {
  consumeArrayOfTkToTv($arrayOfInt);
}
