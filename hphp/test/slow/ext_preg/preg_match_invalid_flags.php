<?hh

<<__EntryPoint>>
function main_preg_match_invalid_flags() :mixed{
  var_dump(preg_match('/test/', 'Hello World', 0xdead));
}
