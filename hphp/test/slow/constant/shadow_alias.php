<?hh

const X = 1;
const hello = 'hello';
use const hello as x;

<<__EntryPoint>> function main(): void {
  @var_dump(X);
  @var_dump(x);
}
