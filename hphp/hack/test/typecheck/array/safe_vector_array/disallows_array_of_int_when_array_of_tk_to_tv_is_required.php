<?hh // partial

function consumeArrayOfTkToTv<Tk, Tv>(array<Tk, Tv> $_): void {}

function test(array<int> $arrayOfInt): void {
  consumeArrayOfTkToTv($arrayOfInt);
}
