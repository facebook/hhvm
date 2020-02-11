<?hh

class E {
   private function f() {}
   function __call($name, $args) {}
}

<<__EntryPoint>> function main(): void {
$isCallable = is_callable(varray['E', 'f']);
var_dump($isCallable);
}
