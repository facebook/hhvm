<?hh /* destructor */

class A {
  public static function foo(...$args) :mixed{
   var_dump('failed');
   return 12;
  }
}

class lol {}
class B {
  public static function bar($_1, $_2, inout $_3) :mixed{
    var_dump(vec[$_1, $_2, $_3]);
    $x = new lol();
    return shape('value' => $x);
  }
}

function main() :mixed{
  var_dump(A::foo(1,2));
  $l = A::foo(1,2);
}



<<__EntryPoint>>
function main_member_fn_intercept() :mixed{
fb_intercept2('A::foo', 'B::bar');

main();
echo "done\n";
}
