<?hh

function f(int $i1, int $i2 = 4, int ...$is): void {}

function g(): void {
  f(...vec[]); // can't splat array when there are required arguments
  f(3, ...vec[]); // ok
}

function ff(int $i2 = 4): void {}

function gg(): void {
  ff(...vec[]); // error. Ok at runtime but not safe
  ff(...vec[2,3]); // because the list could look like this
}
