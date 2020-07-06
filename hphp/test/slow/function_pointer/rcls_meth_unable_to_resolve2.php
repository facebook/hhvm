<?hh

final class Test {}

<<__EntryPoint>>
function main(): void {
  $a = Test::notfoundmethod<int>;
  var_dump($a);
}
