<?hh // partial
function foo1(Vector $c): void {
  $c[] = 'a';
}
function foo2(Map $c, Pair $p): void {
  $c[] = $p;
}
function foo3(Map $c, Pair<int, string> $p): void {
  $c[] = $p;
}
function foo4(Map $c): void {
  $c[] = Pair {42, 'b'};
}
function foo5(Set $c): void {
  $c[] = 73;
}
