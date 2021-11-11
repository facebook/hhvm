<?hh

class C {
  public dict<int, int> $d = dict[];
}

function test_dict_key(vec<~int> $vli, vec<~string> $vls, dynamic $d) : void {
  $li = $vli[0];
  $ls = $vls[0];
  $c = new C();

  hh_expect_equivalent<~int>($c->d[$li]);
  hh_expect_equivalent<~int>($c->d[$ls]);
  hh_expect_equivalent<~int>($c->d[$d]);
  hh_expect_equivalent<~int>($c->d[1]);

  $c->d[$li] = 1;
  $c->d[$d] = 1;

  $d2 = dict<int, int>[];
  $d2[$ls] = 1;
  hh_expect_equivalent<dict<(int | string), int>>($d2);

  $a = dict[$d=>1];
  hh_expect_equivalent<dict<arraykey, int>>($a);
  $b = dict[$li=>1];
  hh_expect_equivalent<dict<arraykey, int>>($b);
  $c = dict[$ls=>1];
  hh_expect_equivalent<dict<arraykey, int>>($c);
  $d = dict[$ls=>1, $li=>1];
  hh_expect_equivalent<dict<arraykey, int>>($d);
}
