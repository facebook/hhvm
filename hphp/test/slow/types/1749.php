<?hh

class X {
}
function bug() {
  if (!$GLOBALS['x']) {
    return;
  }
  return new X;
}

<<__EntryPoint>>
function main_1749() {
;
var_dump(bug());
}
