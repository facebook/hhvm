<?hh

function foo(): vec<int> {
  return vec[
    /* HH_FIXME[4110] 2 */
    100, '2', 300,
    400, 500, 600,
    /* HH_FIXME[4110] 8 */
    700, '8', 900,
  ];
}
