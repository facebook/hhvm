<?hh

function make(bool $null): ?stdclass {
  return $null ? null : new stdclass();
}

function boom(bool $null): <<__Soft>> stdclass {
  return make($null);
}

function gone(inout bool $null): <<__Soft>> stdclass {
  $result = make($null);
  $null = false;
  return $result;
}

<<__EntryPoint>>
function test(): void {
  var_dump(boom(false));
  var_dump(boom(true));
  $inout = true;
  var_dump(gone(inout $inout));
}
