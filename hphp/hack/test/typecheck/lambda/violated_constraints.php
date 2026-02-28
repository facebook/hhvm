<?hh

class A<T as num> {}

function test() : void {
  // constraints violated here in two places
  $f = (A<string> $x) : A<string> ==> { return $x;};
}
