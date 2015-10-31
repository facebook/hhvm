<?hh //strict

/* For now, we don't grow tuple-like arrays - appending to them just downgrades
 * them to vector-like arrays */

function test(): void {
  $a = array(4, 'aaa');
  $a[] = true;
  hh_show($a);

  $a = Vector {array(4, 'aaa')};
  $a[0][] = true;
  hh_show($a[0]);
}
