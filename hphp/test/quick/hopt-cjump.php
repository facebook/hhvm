<?hh

function same($left, $right) :mixed{
  echo ($left === $right) ? "true\n" : "false\n";
}

function eq($left, $right) :mixed{
  echo (HH\Lib\Legacy_FIXME\eq($left, $right)) ? "true\n" : "false\n";
}

function neq($left, $right) :mixed{
  echo (HH\Lib\Legacy_FIXME\neq($left, $right)) ? "true\n" : "false\n";
}
<<__EntryPoint>> function main(): void {
same(false, 0);
neq(0, "b");
eq(true, -1);
}
