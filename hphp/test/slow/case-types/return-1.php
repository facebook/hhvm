<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}
class C {}

case type C1 = int | string;
case type C2 = bool | A;
case type C3 = B | C2;

function foo1(mixed $x):C1 { return $x; }
function foo2(mixed $x):C2 { return $x; }
function foo3(mixed $x):C3 { return $x; }

function format($arg):string {
  if ($arg is bool) return $arg ? "true" : "false";
  if ($arg is string) return "\"".$arg."\"";
  if ($arg is num) return (string)$arg;
  return get_class($arg);
}

<<__EntryPoint>>
function main():void {
  require "test.inc";
  throw_errors();

  $funcs = vec[foo1<>, foo2<>, foo3<>];
  $args = vec[1, "foo", true, 1.1, new A, new B, new C];

  foreach ($funcs as $func) {
    foreach ($args as $arg) {
      $arg_str = format($arg);
      echo "calling ".var_export($func, true)."($arg_str): ";
      try {
        $func($arg);
        echo "OK";
      } catch (Exception $e) {
        echo $e->getMessage();
      }
      echo "\n";
    }
  }
}
