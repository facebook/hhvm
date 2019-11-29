<?hh

enum Enum : string {
  V1 = 'val1';
  V2 = 'val2';
  V3 = 'val3';
}

type Alias = Enum;

record A {
  Alias x;
}
<<__EntryPoint>> function main(): void {
$a = A['x' => Enum::V2];
var_dump($a['x']);
$a['x'] = 'val2';
var_dump($a['x']);
$a['x'] = 1;
var_dump($a['x']);
}
