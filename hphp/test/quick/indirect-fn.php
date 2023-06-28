<?hh
<<__DynamicallyCallable>>
function f($a) :mixed{
  echo $a;
  echo "\n";
}
<<__EntryPoint>> function main(): void {
$name = "f";
$name("param");
}
