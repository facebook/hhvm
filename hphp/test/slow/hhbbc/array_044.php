<?hh

class C { function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{
  return mt_rand() ? vec[new C, new C] : vec[new C, new C, new C];
}
function bar() :mixed{
  $x = foo();
  $x[] = new C;
  return $x['nothere'];
}
<<__EntryPoint>>
function main() :mixed{
  try { $x = bar(); } catch (Exception $e) {
    echo $e->getMessage()."\n";
    $x = null;
  }
  if ($x) $x->heh();
}
