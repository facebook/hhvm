<?hh

<<__EntryPoint>>
function main() {
  require_once('compact_serialize.inc');
  set_error_handler(($errno, $errstr) ==> { echo "$errstr\n"; });
  fb_cs_entrypoint();
}
