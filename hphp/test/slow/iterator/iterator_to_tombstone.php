<?hh

<<__EntryPoint>>
function main(): void {
  $a = Map {1 => 11, 2 => 22, 3 => 33};
  $i = $a->getIterator();
  $i->next();
  unset($a[2]);
  \var_dump($i->current());
}
