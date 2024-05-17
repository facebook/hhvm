<?hh

function my_main(): void {
  $items = vec[1, 2, 3];
  foreach ($items as $item) {
    try {
    } finally {
      continue;
      break;
    }
  }
}
