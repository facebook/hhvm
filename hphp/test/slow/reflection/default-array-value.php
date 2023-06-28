<?hh

function test($k = Map { 'a' => 17, 'b' => 'garbage' }) :mixed{ }

<<__EntryPoint>>
function main() :mixed{
  $c = new ReflectionFunction('test');
  var_dump($c->getParameters()[0]->info['default']);
}
