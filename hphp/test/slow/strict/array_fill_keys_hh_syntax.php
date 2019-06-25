<?hh
<<__EntryPoint>> function main(): void {
$keys = array(
  1234,
  'foo',
  1.234,
);

var_dump(array_fill_keys($keys, 'foo'));
}
