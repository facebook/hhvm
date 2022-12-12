<?hh

class C {}

function r ((C & dynamic) $k) : ?((C & dynamic) | int){
  return $k;
}
