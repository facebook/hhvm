<?hh // experimental

function foo(int $i): void {
  let ret = -1;
  switch ($i) {
    case 1:
      var_dump(ret);
      let ret = 1;
      var_dump(ret);
      // FALLTHROUGH
    case 2:
      var_dump(ret);
      let ret = 2;
      var_dump(ret);
      return ret;
    default:
      var_dump(ret);
      let ret = 42;
      var_dump(ret);
      return ret;
  }
}



<<__EntryPoint>>
function main_switch() {
var_dump(foo(1));
var_dump(foo(2));
var_dump(foo(42));
}
