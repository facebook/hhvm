<?hh

class MyInt {}
class MyBool {}

case type IntOrBool<T> = int where T super MyInt | bool where T super MyBool;

function test<<<__Enforceable>> reify Tgen as arraykey, Tret>(
  IntOrBool<Tret> $x,
): Tret {
  if ($x is Tgen) {
    return new MyInt();
  } else {
    return new MyBool(); // expect error
  }
}
