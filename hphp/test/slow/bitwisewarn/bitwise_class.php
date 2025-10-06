<?hh

class C {}

<<__EntryPoint>>
function main():void {
  $s = HH\classname_to_class(C::class);
  var_dump($s); var_dump($s | "m");
}
