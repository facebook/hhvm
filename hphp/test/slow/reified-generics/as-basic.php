<?hh

function f<reify T>(mixed $x) :mixed{
  try {
    $x as T;
    var_dump("yes");
  } catch (Exception $_) {
    var_dump("nope");
  }
}
<<__EntryPoint>> function main(): void {
f<int>("hello");
f<int>(1);
}
