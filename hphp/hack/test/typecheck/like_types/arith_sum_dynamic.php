<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($d + $i);
  hh_show($d + $li);
  hh_show($d + $f);
  hh_show($d + $lf);
  hh_show($d + $n);
  hh_show($d + $ln);
  hh_show($d + $d);
}
