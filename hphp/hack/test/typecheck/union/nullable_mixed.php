<?hh

function myidx<T>(T $_): ?T {
  return null;
}

function mymain(): void {
  hh_log_level('show', 3);
  $val = myidx<mixed>(0);
  hh_show($val);
}
