<?hh // partial
/* HH_FIXME[4101] */
function foo1(Vector $c): void {
  $c[] = 'a';
}
/* HH_FIXME[4101] */
function foo2(Map $c, Pair $p): void {
  $c[] = $p;
}
/* HH_FIXME[4101] */
function foo3(Map $c, Pair<int, string> $p): void {
  $c[] = $p;
}
/* HH_FIXME[4101] */
function foo4(Map $c): void {
  $c[] = Pair {42, 'b'};
}
/* HH_FIXME[4101] */
function foo5(Set $c): void {
  $c[] = 73;
}
