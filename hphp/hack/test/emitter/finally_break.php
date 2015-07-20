<?hh // strict

function break_finally_0(int $b): int {
  while (true) {
    try {
      if ($b === 0) {
        break;
      }
    } finally {
      echo "finally\n";
    }
    return 1;
  }
  return 0;
}

function break_finally_1(int $b): int {
  while (true) {
    try {
      if ($b === 0) {
        break;
      } else if ($b === 1) {
        echo "continuing\n";
        $b++;
        continue;
      } else if ($b === 2) {
        return 5;
      }
    } finally {
      echo "finally\n";
    }
  }
  return 0;
}

function break_finally_2(int $b): int {
  while ($b != 343) {
    try {
      if ($b >= 300) { $b++; echo $b; continue; }

      try {
        if ($b === 0) {
          break;
        } else if ($b === 1) {
          return 42;
        }
      } finally {
        echo "finally\n";
      }
    } finally {
      echo "more finally\n";
    }
    if ($b == 10) break;
  }
  return 1;
}

function test(): void {
  var_dump(break_finally_0(0));
  var_dump(break_finally_0(1));

  var_dump(break_finally_1(0));
  var_dump(break_finally_1(1));
  var_dump(break_finally_1(2));

  var_dump(break_finally_2(0));
  var_dump(break_finally_2(1));
  var_dump(break_finally_2(340));
  var_dump(break_finally_2(10));
}
