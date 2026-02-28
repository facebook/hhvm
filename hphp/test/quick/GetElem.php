<?hh

<<__EntryPoint>>
function main(): void {
  error_reporting(0);

  $a = vec[0, "b", "c"];

  print $a[0];
  $a[0] = 1;
  print $a[0];
  print $a[0]++;
  print $a[0];
  print "\n";

  $a[3]++;
}
