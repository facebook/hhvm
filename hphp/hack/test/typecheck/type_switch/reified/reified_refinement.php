<?hh

class MyReifiedClass<reify T> {
}

final class MyIntRefiedClass extends MyReifiedClass<int> {}

function check_disjointness(MyReifiedClass<int> $x): void {
  if ($x is MyReifiedClass<string>) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<MyReifiedClass<int>>($x);
  }
  if ($x is MyReifiedClass<int>) {
    hh_expect_equivalent<MyReifiedClass<int>>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
  if ($x is MyReifiedClass<arraykey>) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<MyReifiedClass<int>>($x);
  }
}

function check_no_relation_and_env_update<reify T>(
  MyReifiedClass<T> $x,
  T $t,
): void {
  if ($x is MyReifiedClass<string>) {
    hh_expect_equivalent<MyReifiedClass<T>>($x);
    hh_expect_equivalent<MyReifiedClass<string>>($x);
    hh_expect<string>($t);
  } else {
    hh_show($x); // ideally: MyReifiedClass<T> & not MyReifiedClass<string>
  }
}
