<?hh

function test(int $i, ~int $li, float $f, ~float $lf, num $n, ~num $ln, dynamic $d): void {
  hh_show($i / $i);
  hh_show($i / $li);
  hh_show($i / $f);
  hh_show($i / $lf);
  hh_show($i / $n);
  hh_show($i / $ln);
  hh_show($i / $d);
}
