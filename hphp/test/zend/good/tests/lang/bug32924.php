<?hh
<<__EntryPoint>> function main(): void {
  include_once(dirname(__FILE__).'/bug32924.inc');
  require_once(dirname(__FILE__).'/bug32924.inc');

  foo();
  echo "END";
}
