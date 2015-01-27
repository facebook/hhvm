<?hh // strict

function f(): int {
  while (true) {
    return 1;
  }
}

function g(): int {
  do {
  } while (true);
}

function h(): int {
  for (; ; ) {
    return 1;
  }
}

function i(): int {
  for (; ; ) {
    for (; ; ) {
      break;
    }
  }
}
