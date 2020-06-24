<?hh

class X {
}
function bug() {
  if (!\HH\global_get('x')) {
    return;
  }
  return new X;
}

<<__EntryPoint>>
function main_1749() {
;
var_dump(bug());
}
