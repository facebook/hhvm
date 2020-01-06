<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($n / $i);
  hh_show($n / $li);
  hh_show($n / $f);
  hh_show($n / $lf);
  hh_show($n / $n);
  hh_show($n / $ln);
  hh_show($n / $d);
}
