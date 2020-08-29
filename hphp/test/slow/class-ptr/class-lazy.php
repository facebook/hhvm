<?hh

class Bar {}

<<__EntryPoint>>
function main() {
  $c = Bar::class;
  var_dump($c);
}
