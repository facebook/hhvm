<?hh
function f(int $i) {
  var_dump($i);
}
function g(string $s) {
  var_dump($s);
}


<<__EntryPoint>>
function main_2189() {
g(DATE_RFC850);
f(count(varray[]));
}
