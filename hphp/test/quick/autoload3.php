<?hh
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'B') {
    include 'autoload3-1.inc';
  } else if ($cls === 'I') {
    include 'autoload3-2.inc';
  } else if ($cls === 'j') {
    include 'autoload3-3.inc';
  } else if ($cls === 'K') {
    include 'autoload3-4.inc';
  } else if ($cls === 'l') {
    include 'autoload3-5.inc';
  }
}

class C extends B implements I, j {
}

interface P extends K, l {
}

$obj = new C;
echo "Done\n";

