<?hh // strict
class C {}


function make(): C {
  // OK
  return new C();
}
