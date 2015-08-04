<?hh

function f(array $partial, array<int> $checked1, array<string, int> $checked2) {
  $a = $partial;
  $a = $checked1;
  $a = $checked2;
}
