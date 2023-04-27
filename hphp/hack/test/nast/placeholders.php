<?hh

function placeholder_param(int $_): void {}

function returns_int() : int { return 0; }

function placeholder_local(): void {
  $_ = returns_int();
}

function placeholder_foreach_val(vec<int> $xs): void {
  foreach($xs as $_) {}
}

function placeholder_foreach_keyval_val(dict<string,int> $xs): void {
  foreach($xs as $k => $_) {}
}

function placeholder_foreach_keyval_key(dict<string,int> $xs): void {
  foreach($xs as $_ => $v) {}
}

function placeholder_foreach_keyval(dict<string,int> $xs): void {
  foreach($xs as $_ => $_) {}
}
