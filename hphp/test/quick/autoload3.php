<?php
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'B') {
    class b {}
  } else if ($cls === 'I') {
    interface i {}
  } else if ($cls === 'j') {
    interface j {}
  } else if ($cls === 'K') {
    interface K {}
  } else if ($cls === 'l') {
    interface L {}
  }
}

class C extends B implements I, j {
}

interface P extends K, l {
}

$obj = new C;
echo "Done\n";

