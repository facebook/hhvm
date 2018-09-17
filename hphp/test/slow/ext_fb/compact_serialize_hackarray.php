<?hh

require_once('compact_serialize.inc');

<<__EntryPoint>>
function main() {
  set_error_handler(($errno, $errstr) ==> { echo "$errstr\n"; });
  fb_cs_entrypoint();
}
