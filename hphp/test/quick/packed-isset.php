<?hh

function foo() :mixed{
  $arr = dict[];
  for ($i = 0; $i < 4; $i++) {
    $arr[$i] = $i;
  }
  for ($i = 0; $i < 5; $i++) {
    print((string)(isset($arr[2]))."\n");
  }
  for ($i = 0; $i < 5; $i++) {
    print((string)(isset($arr[$i]))."\n");
  }
}

function foo2() :mixed{
  $arr = dict[];
  for ($i = 0; $i < 4; $i++) {
    $arr[$i] = null;
  }
  for ($i = 0; $i < 5; $i++) {
    var_dump(isset($arr[2]));
  }
  for ($i = 0; $i < 5; $i++) {
    var_dump(isset($arr[$i]));
  }
}
<<__EntryPoint>>
function main_entry(): void {

  foo();

  foo2();
}
