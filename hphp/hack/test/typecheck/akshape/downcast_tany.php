<?hh
/**
 * Shape-like array inference algorithm is best-effort, and it's fairly easy
 * to confuse it to give up and treat array as map-like, even when it was not
 * the developer intention. When this happens, we should adhere to our existing
 * heuristics for map-like array construction.
 * See test/typecheck/array_heterogeneous.php
 */
function test(): void {
  $a = array('x' => 4, 'y' => unknown_class());

  $key = 'y';
  $value = $a[$key];
  $a[$key] = $value;

  $a['y']->functionThatMightExist(); // this should be allowed
}
