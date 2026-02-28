<?hh

class MyReifiedClass<reify T> {
}

final class MyIntRefiedClass extends MyReifiedClass<int> {}

function check_disjoint_due_to_inheritance(MyIntRefiedClass $x): void {
  if ($x is MyReifiedClass<string>) {
    hh_expect_equivalent<nothing>($x);
  } else if ($x is MyReifiedClass<int>) {
    hh_expect_equivalent<MyIntRefiedClass>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}
