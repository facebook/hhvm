<?hh

function foo(): vec<int> {
  return vec[
    100, 200, 300,
    /* HH_FIXME[4110] */
    400, '5', 600,
    700, 800, 900,
  ];
}
