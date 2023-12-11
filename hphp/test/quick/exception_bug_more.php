<?hh

function thrower() :mixed{
  throw new Exception("Not MyJunk");
}

function foo() :mixed{
  foreach (vec[1,2,3] as $x) {
    try {
      thrower();
    } catch (MyJunk $z) { echo "Not Here\n"; }
  }
}
<<__EntryPoint>> function main(): void {
try {
  foo();
} catch (Exception $x) { echo "done\n"; }
}
