<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($ln / $i);
  hh_show($ln / $li);
  hh_show($ln / $f);
  hh_show($ln / $lf);
  hh_show($ln / $n);
  hh_show($ln / $ln);
  hh_show($ln / $d);
}
