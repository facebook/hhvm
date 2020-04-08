<?hh

function f(UndefClass $x) {}

<<__EntryPoint>> function main(): void {
  f(new stdClass());
}
