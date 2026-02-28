<?hh

class C {}

<<__EntryPoint>>
function main():void {
  $s = C::class;
  var_dump($s); var_dump($s | "m");
}
