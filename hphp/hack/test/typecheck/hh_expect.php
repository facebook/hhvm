<?hh

function f(int $i) : void {
  hh_expect<int>($i);
  hh_expect<mixed>($i);
  hh_expect<nothing>($i);
  hh_expect<string>($i);


  hh_expect_equivalent<int>($i);
  hh_expect_equivalent<mixed>($i);
  hh_expect_equivalent<nothing>($i);
  hh_expect_equivalent<string>($i);
}
