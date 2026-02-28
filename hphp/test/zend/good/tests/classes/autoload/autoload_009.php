<?hh

function f(UndefClass $x) :mixed{}

<<__EntryPoint>> function autoload_009(): void {
  f(new stdClass());
}
