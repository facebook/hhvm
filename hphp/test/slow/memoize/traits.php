<?hh

<<__EntryPoint>>
function entrypoint_traits(): void {
  require_once(__DIR__.'/namespacefoo.php.inc');
  require_once(__DIR__.'/namespacebar.php.inc');
  require_once(__DIR__.'/traits.inc');

  echo "basic trait use\n";

  basic_test(C::class, 'test');

  echo "override method\n";

  basic_test(CO::class, 'test');

  echo "override meth with different sig\n";

  basic_test(CDS::class, 'test');
  $c = new CDS();
  echo $c->test(5).' ';
  echo $c->test(5).' ';
  $c = new CDS();
  echo $c->test(5).' ';
  echo $c->test(5)."\n";

  echo "override meth with different sig with namespaces\n";

  basic_test(CN::class, 'test');
  $c = new CN();
  echo $c->test(3).' ';
  echo $c->test(3).' ';
  $c = new CN();
  echo $c->test(3).' ';
  echo $c->test(3)."\n";

  echo "static trait method\n";

  echo CS1::testTraitStatic().' ';
  echo CS1::testTraitStatic().' ';
  echo CS2::testTraitStatic().' ';
  echo CS2::testTraitStatic();
}
