<?hh

<<__EntryPoint>>
function entrypoint_autoload_get_paths(): void {
  print "Paths:\n";
  $autoload_paths = HH\autoload_get_paths();
  \sort(inout $autoload_paths);
  foreach ($autoload_paths as $path) {
    print "  $path\n";
  }
}
