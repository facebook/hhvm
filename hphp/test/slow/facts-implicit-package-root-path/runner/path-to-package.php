<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(HH\Facts\path_to_package('member/file.inc'));
  var_dump(HH\Facts\path_to_package('missing/file.inc'));
}
