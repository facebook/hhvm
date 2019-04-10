<?hh //strict

/* For now, we don't grow tuple-like arrays - appending to them just downgrades
 * them to vector-like arrays */

enum E : int { A = 2; };

function expectArrayAK(array<arraykey> $a):void { }

function test(): void {
  $a = array(4, 'aaa');
  $a[] = E::A;
  expectArrayAK($a);

  $a = Vector { array(4, 'aaa') };
  $a[0][] = E::A;
}
