<?hh

class A {
 private $b = 10;
 }
class B extends A {
 private $b = 100;
 }

<<__EntryPoint>>
function main_1815() :mixed{
apc_store('key', new B());
var_dump(__hhvm_intrinsics\apc_fetch_no_check('key'));
}
