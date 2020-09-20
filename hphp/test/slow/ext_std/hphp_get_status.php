<?hh

<<__EntryPoint>>
function main(): void {
  $status = hphp_get_status();
  var_dump(is_darray($status));
}
