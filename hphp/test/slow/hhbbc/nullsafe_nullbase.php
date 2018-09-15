<?hh

function err() { throw new exception; }

class somecls {
 public function x(&$k) {}
}

function foo(somecls $z) {
 try {
 $z->x(&$y?->z);
 } catch (exception $e) {
 set_error_handler(null);
 var_dump($y);
 }
}

<<__EntryPoint>>
function main_nullsafe_nullbase() {
set_error_handler('err');

foo(new somecls);
}
