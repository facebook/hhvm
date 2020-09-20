<?hh

<<__EntryPoint>>
function main(): void {
  $a = Map {1 => 11, 2 => 22, 3 => 33};
  $i = $a->getIterator();
  unset($a[2]);
  $i->next();
  \var_dump($i->current());
}
