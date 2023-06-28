<?hh

class X {
}
function bug() :mixed{
  if (!\HH\global_get('x')) {
    return;
  }
  return new X;
}

<<__EntryPoint>>
function main_1749() :mixed{
;
var_dump(bug());
}
