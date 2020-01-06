<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($li + $i);
  hh_show($li + $li);
  hh_show($li + $f);
  hh_show($li + $lf);
  hh_show($li + $n);
  hh_show($li + $ln);
  hh_show($li + $d);
}
