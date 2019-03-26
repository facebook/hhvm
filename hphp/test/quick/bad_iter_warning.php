<?hh

function main() {
  $arr = null;
  print "start iter loop\n";
  foreach ($arr as $x => $y) {
    print "fail";
  }
  print "end iter loop\n";
}

main();

