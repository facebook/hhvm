<?hh

class X {
  const FOO = Y::BAR;
  const BAZ = 5;
  const BIZ = Y::BAR;
}

<<__EntryPoint>>
function main() {
  if (!isset($g)) {
    include 'class_constants-1.inc';
  }

  require_once(__DIR__.'/class_constants-classes1.inc');

  if (!isset($g)) {
    include 'class_constants-2.inc';
  }

  require_once(__DIR__.'/class_constants-classes2.inc');

  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'autoloadedclass' => 'class_constants-3.inc',
      ],
    ],
    __DIR__.'/',
  );

  print "Test begin\n";

  var_dump(X::BAZ);
  var_dump(X::FOO);
  var_dump(Z::FOO);
  var_dump(Z::BIZ);

  var_dump(A::BAR);
  var_dump(A::FOO);
  var_dump(A::BAR);
  var_dump(B::FOO);
  var_dump(B::BAR);
  var_dump(C::FOO);
  var_dump(C::BAR);


  var_dump(K::WEE);
  var_dump(K::WOO);
  var_dump(L::WEE);

  var_dump(constant('K::WEE'));
  var_dump(constant('K::WOO'));
  var_dump(constant('L::WEE'));

  # Test forcing autoload.
  var_dump(AutoloadedClass::INDEED);

  print "Test end\n";
  var_dump(D::FakeConstant);

}
