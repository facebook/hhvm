<?hh

class C { function heh() { echo "heh\n"; } }
function foo() {
  return mt_rand() ? array(new C, new C) : array(new C, new C, new C);
}
function bar() {
  $x = foo();
  $x['a'] = new C;
  return $x['nothere'];
}
<<__EntryPoint>>
function main() {
  try { $x = bar(); } catch (Exception $e) {
    echo $e->getMessage()."\n";
    $x = null;
  }
  if ($x) $x->heh();
}
