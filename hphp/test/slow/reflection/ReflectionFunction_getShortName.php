<?hh

namespace foo\bar;

abstract final class FStatics {
  public static $staticX = 4;
  public static $staticY;
}

#===============================================================================
# ReflectionFunction.

function f($a, inout $b, $c=null) {
  print "In f()\n";
  FStatics::$staticX++;
  $x = FStatics::$staticX;
  return $x;
}

class Test {
  public function test() {
  }
}



<<__EntryPoint>>
function main_reflection_function_get_short_name() {
$rf = new \ReflectionFunction('\foo\bar\f');
print "--- getShortName(\"\\foo\\bar\\f\") ---\n";
\var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"\\foo\\bar\\f\") ---\n";
\var_dump($rf->getNamespaceName());
print "\n";

$rf = new \ReflectionMethod('\foo\bar\Test', 'test');
print "--- getShortName(\"\\foo\\bar\\Test::test\") ---\n";
\var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"\\foo\\bar\\Test::test\") ---\n";
\var_dump($rf->getNamespaceName());
print "\n";

$rf = new \ReflectionFunction('\strlen');
print "--- getShortName(\"strlen\") ---\n";
\var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"strlen\") ---\n";
\var_dump($rf->getNamespaceName());
print "\n";
}
