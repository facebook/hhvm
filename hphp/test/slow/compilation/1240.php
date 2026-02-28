<?hh
function bar() :mixed{
  return 123;
}


<<__EntryPoint>>
function main_1240() :mixed{
  $a = bar();
  if ($a) {
    include '1240-1.inc';
  } else {
    include '1240-2.inc';
  }
  fOO();
}
