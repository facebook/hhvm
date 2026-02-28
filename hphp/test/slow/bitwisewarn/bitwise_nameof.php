<?hh

class C {}

<<__EntryPoint>>
function main():void {
  $s = nameof C;
  var_dump($s); var_dump($s & "m");
}
