<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($f / $i);
  hh_show($f / $li);
  hh_show($f / $f);
  hh_show($f / $lf);
  hh_show($f / $n);
  hh_show($f / $ln);
  hh_show($f / $d);
}
