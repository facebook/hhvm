<?hh
<<__EntryPoint>> function main(): void {
$array = darray['k1' => darray[]];

$it = new RecursiveIteratorIterator(
  new RecursiveArrayIterator($array)
);

for (true; $it->valid(); $it->next()) {
  var_export(
    darray[
      'key' => $it->key(),
      'value' => $it->current(),
    ]
  );
}
print("\n===DONE===\n");
}
