<?hh

function err() :mixed{ throw new Exception; }

class somecls {
 public function x(inout $k) :mixed{}
}

function foo(somecls $z) :mixed{
 try {
 $z->x(inout $y?->z);
 } catch (Exception $e) {
 set_error_handler(null);
 var_dump($y);
 }
}

<<__EntryPoint>>
function main_nullsafe_nullbase() :mixed{
set_error_handler(err<>);

foo(new somecls);
}
