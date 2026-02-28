<?hh

function test1() {
  var_dump(__METHOD__);
}

<<__EntryPoint>>
function main() {
fb_rename_function('test1', 'fiz');
}
