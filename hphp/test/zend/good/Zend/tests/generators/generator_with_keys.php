<?hh

function reverse(darray $array) :AsyncGenerator<mixed,mixed,void>{
  $keys = vec[];
  $vals = vec[];
  foreach ($array as $key => $val) {
    $keys[] = $key;
    $vals[] = $val;
  }
  for ($i = count($array) - 1; $i >= 0; $i--) {
    yield $keys[$i] => $vals[$i];
  }
}
<<__EntryPoint>> function main(): void {
  $array = dict['foo' => 'bar', 'bar' => 'foo'];
  foreach (reverse($array) as $key => $value) {
    echo $key, ' => ', $value, "\n";
  }
}
