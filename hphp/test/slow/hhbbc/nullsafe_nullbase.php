<?hh

function err() { throw new Exception; }

class somecls {
 public function x(inout $k) {}
}

function foo(somecls $z) {
 try {
 $z->x(inout $y?->z);
 } catch (Exception $e) {
 set_error_handler(null);
 var_dump($y);
 }
}

<<__EntryPoint>>
function main_nullsafe_nullbase() {
set_error_handler(err<>);

foo(new somecls);
}
