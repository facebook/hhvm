<?hh

// This error handler swallows typehint errors, which is
// disallowed in RepoAuthoritative mode. Thus, this test
// is set to be norepo.
function err($code, $msg) {
  echo "Handled ${code}: $msg", "\n";
  return true;
}
set_error_handler('err');

class CExplicit implements Stringish {
  public function __toString() {
    return __CLASS__;
  }
}

class CImplicit {
  public function __toString() {
    return __CLASS__;
  }
}

trait TStringish {
  public function __toString() { return __TRAIT__; }
}

interface IStringish {
  public function __toString();
}

class CThruTrait {
  use TStringish;
}

function f1(?Stringish $x): void {
  $s = Stringish::class;
  echo ($x instanceof Stringish) ? "true" : "false", ", ";
  echo ($x instanceof $s) ? "true" : "false", ", ";
  var_dump($x);
  echo "\n";
}

function f2(Stringish $x): void {
  $s = Stringish::class;
  echo ($x instanceof Stringish) ? "true" : "false", ", ";
  echo ($x instanceof $s) ? "true" : "false", ", ";
  var_dump($x);
  echo "\n";
}

function test_functionality() {
  echo '********** static string **********', "\n";
  f1("a boring string");
  f2("a boring string");

  echo '********** dynamic string **********', "\n";
  $x = "hello ";
  if (time() > 0) {
    $x .= "world";
  }
  f1($x);
  f2($x);

  echo '********** explicit implements **********', "\n";
  $explicit = new CExplicit();
  f1($explicit);
  f2($explicit);

  echo '********** implicit implements **********', "\n";
  $implicit = new CImplicit();
  f1($implicit);
  f2($implicit);

  echo '********** via trait implements **********', "\n";
  $via_trait = new CThruTrait();
  f1($via_trait);
  f2($via_trait);

  echo '********** null **********', "\n";
  f1(null);
  f2(null);

  echo '********** array **********', "\n";
  f1(array(1, 2, 3));
  f2(array(1, 2, 3));

  echo '********** number **********', "\n";
  f1(10);
  f2(10);
  f1(-4.2);
  f2(-4.2);
}

function test_reflection() {
  echo "\n",
    '--------------------', ' ', __FUNCTION__, ' ', '--------------------',
    "\n\n";
  var_dump(interface_exists(Stringish::class));

  var_dump(is_a(CExplicit::class, Stringish::class));
  var_dump(is_subclass_of(CExplicit::class, Stringish::class));

  var_dump(is_a(CImplicit::class, Stringish::class));
  var_dump(is_subclass_of(CImplicit::class, Stringish::class));

  $rc = new ReflectionClass(CExplicit::class);
  var_dump($rc->getInterfaceNames());
  var_dump($rc->implementsInterface(Stringish::class));
  var_dump($rc->isSubclassOf(Stringish::class));

  $rc = new ReflectionClass(CImplicit::class);
  var_dump($rc->getInterfaceNames());
  var_dump($rc->implementsInterface(Stringish::class));
  var_dump($rc->isSubclassOf(Stringish::class));

  $rc = new ReflectionClass(CThruTrait::class);
  var_dump($rc->getInterfaceNames());
  var_dump($rc->implementsInterface(Stringish::class));
  var_dump($rc->isSubclassOf(Stringish::class));

  $rc = new ReflectionClass(TStringish::class);
  var_dump($rc->getInterfaceNames());
  var_dump($rc->implementsInterface(Stringish::class));
  var_dump($rc->isSubclassOf(Stringish::class));

  $rc = new ReflectionClass(IStringish::class);
  var_dump($rc->getInterfaceNames());
  var_dump($rc->implementsInterface(Stringish::class));
  var_dump($rc->isSubclassOf(Stringish::class));

}

test_functionality();
test_reflection();
