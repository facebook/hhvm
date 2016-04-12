<?hh //strict

/**
 * Shape like array + map like array = map-like array
 * - usage that should report errors.
 */

function test(array<string, int> $b): void {

  $a = array();
  $a['aaa'] = 3.14;

  $c = $a + $b;
  take_int_array($c);
}

function take_int_array<Tk>(array<Tk, int> $_): void {}
