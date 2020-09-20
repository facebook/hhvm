<?hh // strict

function f(bool $b, vec<int> $v, vec<string> $v2): void {
  hh_show($b ? $v : vec[]);
  hh_show($b ? vec[] : vec[]);
  hh_show($b ? vec[1] : vec[]);
  hh_show($b ? $v : $v2);
  hh_show($b ? vec[1] : vec['']);
  hh_show($b ? $v : vec['']);
}
