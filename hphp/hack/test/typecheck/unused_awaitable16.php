<?hh // strict

async function f(): Awaitable<string> {
  return 'hehe';
}

async function g(): Awaitable<void> {
  $gens = array('a' => f(), 'b' => f(), 'c' => f());
  $gens = array_map($item ==> (bool) $item, $gens);
}
