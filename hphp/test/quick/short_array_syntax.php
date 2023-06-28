<?hh
function f($x = varray[1]) :mixed{
  return $x;
}
class C {
  public static $y = varray[2, 3];
}
<<__EntryPoint>> function main(): void {
var_dump(varray[]);
var_dump(f());
var_dump(C::$y);
}
