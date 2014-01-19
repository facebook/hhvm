<?php
function f($pass) {
  echo "Begin f($pass)\n";
  foreach (['null','NULL','SID'] as $k) {
    echo "--$k--\n";
    var_dump(defined($k));
    var_dump(constant($k));
  }
  echo "null: "; var_dump(null);
  echo "NULL: "; var_dump(NULL);
  echo "SID: ";  var_dump(SID);
  echo "End f($pass)\n";
}

function main() {
  f(1);
  session_start();
  f(2);
  $s1 = SID;
  session_regenerate_id();
  f(3);
  $s2 = SID;
  var_dump($s1 === $s2);
}

main();
