<?hh

function err() { throw new exception; }
set_error_handler('err');

class somecls {
 public function x(&$k) {}
}

function foo(somecls $z) {
 try {
 $z->x($y?->z);
 } catch (exception $e) {
 set_error_handler(null);
 var_dump($y);
 }
}

foo(new somecls);
