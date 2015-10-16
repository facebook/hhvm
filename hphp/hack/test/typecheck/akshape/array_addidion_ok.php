<?hh //strict

/**
 * Shape like array + map like array = map-like array -  usage with no errors.
 */

function test(array<string, int> $b): void {

  $a = array();
  $a['aaa'] = 3.14;

  $c = $a + $b;
  take_num_array($c);
}

function take_num_array<Tk>(array<Tk, num> $_): void {}
