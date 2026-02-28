<?hh

function one() :mixed{
  echo "one\n";
}

function three() :mixed{
  echo "three\n";
}

abstract final class MyMicrotimeStatics {
  public static $x = 0.0;
}

// Try it with a builtin, too.

function my_microtime(bool $foob = false) :mixed{
  echo "ca\$h m0n3y\n";
  MyMicrotimeStatics::$x += 1.0;
  if (false) {
    return MyMicrotimeStatics::$x;
  }
  return (string)MyMicrotimeStatics::$x;
}

function my_foo() :mixed{}

function bar() :mixed{
  $orig = "foo";
  $new = "my_$orig";
  var_dump(fb_rename_function($new, "foo"));
}
<<__EntryPoint>>
function main_entry(): void {

  one();
  var_dump(fb_rename_function("one", "two"));
  two();
  var_dump(fb_rename_function("three", "one"));
  one();

  var_dump(fb_rename_function('microtime', '__dont_call_microtime'));
  var_dump(fb_rename_function('my_microtime', 'microtime'));
  echo microtime(true) . "\n";

  bar();
}
