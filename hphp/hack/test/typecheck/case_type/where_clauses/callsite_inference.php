<?hh

class MyInt {
  public function intMethod(): void {}
}
class MyString {}

case type CT<T> =
| int where T super MyInt
| string where T super MyString;

function lift<T>(CT<T> $ct): T {
  // haven't implemented the logic to support this implementation yet
  throw new Exception();
}

function takes_my_int(MyInt $x): void {}
function takes_my_string(MyString $x): void {}

function good(): void {
  takes_my_int(lift(1));
  takes_my_string(lift("hello"));
  lift(1)->intMethod();
}

function bad(): void {
  takes_my_int(lift("hello"));
  takes_my_string(lift(1));
}
