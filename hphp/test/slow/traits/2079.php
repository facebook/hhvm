<?hh

trait T {
  public static function m() :mixed{
    echo "original\n";
  }
}

class A {
  use T;
}

class B {
  use T;
}

<<__EntryPoint>>
function main_2079() :mixed{
  T::m();
  A::m();
  fb_intercept2("T::m", function($_1, $_2, inout $_3) {
    echo "new\n";
    return shape('value' => null);
  });
  A::m();
  B::m();
  T::m();
}
