<?hh

namespace {
<<__EntryPoint>> function main(): void {
  $x = new Vector();
  $x[] = 10;
  $x[] = 20;
  $x[] = 30;
  foreach ($x as $i) {
    echo $i . "\n";
  }
}
}
