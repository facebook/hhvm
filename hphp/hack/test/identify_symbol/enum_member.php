<?hh

enum E : int {
  FOO = 4;
}

function test() {
  E::FOO;
}
