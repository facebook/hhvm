<?hh

function main(): void {
  $a = "apple";
  do {
    $a = 1;
    try {
    } finally {
      takes_string($a);
    }
  } while (1 == 2);
}

function takes_string(string $_): void {}
