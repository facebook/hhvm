<?hh

function pess_hint_fun((function(int): int) $f) : (function(int): int)  {
  return $f;
}
