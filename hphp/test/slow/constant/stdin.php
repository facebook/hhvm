<?hh
class A {
  public static function b() :mixed{
    return \HH\stdin();
  }
}

<<__EntryPoint>>
function main_stdin() :mixed{
  var_dump(A::b());
}
