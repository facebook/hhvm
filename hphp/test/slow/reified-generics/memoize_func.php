<?hh
<<__Memoize>>
function f<reify Ta, Tb, reify Tc>($x, $y, $z) :mixed{
  var_dump("hi");
}
<<__Memoize>>
function g<reify Ta, Tb, reify Tc>() :mixed{
  var_dump("hi");
}
<<__EntryPoint>>
function main_entry(): void {

  echo "multiple argument\n";

  f<int, string, bool>(1,2,3); // print
  f<int, string, bool>(1,2,3); // nope
  f<string, string, bool>(1,2,3); // print
  f<string, string, bool>(1,2,3); // nope
  f<string, string, string>(1,2,3); // print
  f<int, string, bool>(1,2,3); // nope
  f<int, string, bool>(1,1,3); // print
  f<int, string, bool>(1,2,3); // nope

  echo "no argument\n";

  g<int, string, bool>(); // print
  g<int, string, bool>(); // nope
  g<string, string, bool>(); // print
  g<string, string, bool>(); // nope
  g<string, string, string>(); // print
  g<int, string, bool>(); // nope
  g<int, string, bool>(); // nope
}
