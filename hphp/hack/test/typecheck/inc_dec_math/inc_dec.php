<?hh

function good_inc_dec(int $a, num $b): void {
  $a++;
  ++$a;
  $b--;
  --$b;
}

function bad_inc_dec(string $a, arraykey $b, bool $c): void {
  --$a;
  $a--;
  ++$b;
  $c++;
}
