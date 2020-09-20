<?hh

function asd($x) {
  var_dump($x);
}

function foo($b) {
  asd($b[]);
}


<<__EntryPoint>>
function main_fpassm_mw() {
foo(varray[]);
}
