<?hh

<<__EntryPoint>> function main(): void {
  $x = 'hey-o';
  $x .= ' world';

  $idx = (5 << 32) | (1 << 15);
  $x[$idx] = 'f';
}
