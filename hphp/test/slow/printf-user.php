<?hh

function myprintf<Targs as (mixed...)>(string $test, HH\TypedFormatString<PlainSprintf,Targs> $fmt, ...Targs $args):void {
  printf("test = %s\n", $test);
  printf($fmt, ...$args);
}
<<__EntryPoint>>
function main():void {
  myprintf("one", "int %d float %f\n", 32, 12.3);
  myprintf("two", "simple\n");
  myprintf("three", "string %s\n", "test");
}
