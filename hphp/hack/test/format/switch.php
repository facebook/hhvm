<?hh

function f($x) {
  switch ($x) {
    case 1: // hi
      break;
    case 2: /* wat */
      break;
    case 3:
      // sup
      break;
    case 4:
      /* but really */
      echo "hi";
      // FALLTHROUGH
    default: // and yet
      break;
  }
}
