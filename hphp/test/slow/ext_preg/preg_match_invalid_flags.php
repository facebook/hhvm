<?hh

<<__EntryPoint>>
function main_preg_match_invalid_flags() {
  var_dump(preg_match_with_matches('/test/', 'Hello World', &$dummy, 0xdead));
}
