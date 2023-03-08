<?hh

function f(UndefClass $x) {}

<<__EntryPoint>> function autoload_009(): void {
  f(new stdClass());
}
