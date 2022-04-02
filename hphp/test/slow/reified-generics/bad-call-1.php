<?hh

function foo<reify T1, T2, T3>($x) { return bar<T1, T2, T3>($x); }
function bar<reify T1, reify T2, T3>($x) { return $x is T2; }

<<__EntryPoint>>
function main() {
  try { foo<int, int, _>(1); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { foo<int, int, _>("ok"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
