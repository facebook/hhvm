<?hh

<<__EntryPoint>>
function example((function(int, int): int) $pick = HH\Lib\SecureRandom\int<>): void {
  $x = $pick(0, 1000);
  print $x;
}
