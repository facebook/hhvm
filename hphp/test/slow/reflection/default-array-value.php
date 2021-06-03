<?hh

function test($k = Map { 'a' => 17, 'b' => 'garbage' }) { }

<<__EntryPoint>>
function main() {
  $c = new ReflectionFunction('test');
  var_dump($c->getParameters()[0]->info['default']);
}
