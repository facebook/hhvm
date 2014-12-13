<?hh

function doh($x, $y) {
  if ($y === 'hey') {
    if ($x === 'exit') {
      global $counter;
      if (($counter++ % 2) == 1) {
        throw new exception('x');
      }
    }
  }
}
fb_setprofile('doh');

function hey() {
  yield new stdclass;
}

for ($i = 0; $i < 3; ++$i) {
  try { foreach (hey() as $k) {} } catch (Exception $x) {}
}

