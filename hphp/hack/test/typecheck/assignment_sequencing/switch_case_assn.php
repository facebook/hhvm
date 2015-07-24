<?hh // strict

function wut() {
  $a = 1;
  switch ($a) {
    case ($a = 0):
      echo "what in the world\n";
      break;
    case 1:
      echo "argh\n";
      break;
  }
}
