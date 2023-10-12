<?hh //strict

/* For now, we don't grow tuple-like arrays - appending to them just downgrades
 * them to vector-like arrays */

enum E : int { A = 2; };

function expectArrayAK(varray<arraykey> $a):void { }

function test(): void {
  $a = varray[4, 'aaa'];
  $a[] = E::A;
  expectArrayAK($a);

  $a = Vector { varray[4, 'aaa'] };
  $a[0][] = E::A;
}
