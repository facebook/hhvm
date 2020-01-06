<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($lf / $i);
  hh_show($lf / $li);
  hh_show($lf / $f);
  hh_show($lf / $lf);
  hh_show($lf / $n);
  hh_show($lf / $ln);
  hh_show($lf / $d);
}
