<?hh

<<__EntryPoint>>
function main() {
  $m = new ReflectionModule('a');
  var_dump($m->getName());
  try {
    new ReflectionModule('b');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
