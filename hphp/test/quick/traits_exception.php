<?hh
<<__EntryPoint>>
function main() {
  require_once(__DIR__."/traits_exception.inc");
  require_once(__DIR__."/traits_exception-2.inc");

  echo XABC::fromClass();
}
