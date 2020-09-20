<?hh // strict

function f(bool $b): int {
  while (true) {
    if ($b) {
      return 1;
    }
  }
}

function g(bool $b): int {
  do {
    if ($b) {
      return 1;
    }
  } while (true);
}

function h(bool $b): int {
  for (; ; ) {
    if ($b) {
      return 1;
    }
  }
}

function i(bool $b): int {
  for (; ; ) {
    for (; ; ) {
      if ($b) {
        return 1;
      }
    }
  }
}
