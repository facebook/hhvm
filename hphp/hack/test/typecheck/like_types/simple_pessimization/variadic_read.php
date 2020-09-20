<?hh

function f(int ...$i): void {
  hh_show($i);
  hh_show($i[0]);
}
