<?hh

<<__EntryPoint>>
function main() {
  \HH\global_set('n', 10);
  $vals = varray[];
  for ($i = 0; $i < 10; $i++) {
    $vals[] = \HH\global_get('n');
  }
  var_dump($vals);

  for ($i = 0; $i < 10; $i++) {
    $gname = "a" . (string)$i;
    \HH\global_set($gname, $i);
  }

  printf("%016x\n", 1 << \HH\global_get('a0'));
  printf("%016x\n", 1 << \HH\global_get('a1'));
  printf("%016x\n", 1 << \HH\global_get('a2'));
  printf("%016x\n", 1 << \HH\global_get('a3'));

  \HH\global_set(42, "---42---");
  $a = varray[];
  $a[] = \HH\global_get(42);
  $a[] = \HH\global_get("42");
  var_dump($a);
}
