<?hh

function f(): void {
  $d = dict['key1' => 42] ?: dict['key2' => 'string'];
  inspect($d);
}
