<?hh

function blah() {
  $xs = varray[1, 2, 3, 4, 5];

  foreach ($xs as $x) {
    try {
      echo "$x\n";
      if ($x == 2) {
        break;
      }
    } finally {
      echo "3\n";
    }
  }
}



<<__EntryPoint>>
function main_finally_break_002() {
blah();
}
