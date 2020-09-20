<?hh

<<__EntryPoint>> function main(): void {
  $arr = null;
  print "start iter loop\n";
  foreach ($arr as $x => $y) {
    print "fail";
  }
  print "end iter loop\n";
}
