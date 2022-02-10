<?hh

function test() {
  if (foo() && bar()) {
    // Empty if should be flagged.
  }
  if (foo() && bar()) {
    print("foo")
  } else {
    // Empty else should be flagged.
  }

  if ($x == null) {
    print("This should be flagged");
  }
  if ($x != null) {
    print("This should be flagged");
  }
  if ($x === null) {
    print("This should not be flagged");
  }
  if (!$x) {
    print("This should be flagged");
  }
}

function empty_isset_test($y) {
  $y = isset($x);
}
