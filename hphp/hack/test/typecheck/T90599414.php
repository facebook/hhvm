<?hh

<<__EntryPoint>>
function non_arraykey_index_access(): void {
  $m = Map {};
  $m[null];
}
