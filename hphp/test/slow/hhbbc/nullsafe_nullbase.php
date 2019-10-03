<?hh

function err() { throw new exception; }

class somecls {
 public function x(inout $k) {}
}

function foo(somecls $z) {
 try {
 $z->x(inout $y?->z);
 } catch (exception $e) {
 set_error_handler(null);
 var_dump($y);
 }
}

<<__EntryPoint>>
function main_nullsafe_nullbase() {
set_error_handler(fun('err'));

foo(new somecls);
}
