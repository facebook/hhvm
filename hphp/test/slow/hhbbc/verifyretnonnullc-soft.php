<?hh

function make(bool $null): ?stdclass {
  return $null ? null : new stdclass();
}

function boom(bool $null): <<__Soft>> stdclass {
  return make($null);
}

<<__EntryPoint>>
function test(): void {
  var_dump(boom(false));
  var_dump(boom(true));
}
