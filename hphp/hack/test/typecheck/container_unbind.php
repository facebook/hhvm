<?hh

type t = (int, string);

function no_type() {}

function test(t $tuple, bool $cond): int {
  $v = Vector {no_type()};

  if ($cond) {
    $tuple = tuple(
      $v[0],
      'aaa',
      4, // this element is here to force creation of an intersection of two
    // tuples. Without it they could just unify into a single one.
    );
  }

  hh_show($tuple);
  /**
   *  $tuple is now:
   *
   * [1]intersect(
   *    ([2]intersect(X), Tstring, Tint),
   *    (Tint, Tstring)
   * )
   **/

  hh_show($tuple[0]); // intersect([2]intersect(X), Tint)
  // assignment to variable is removing the outermost typevars ("unbinds it")
  $x = $tuple[0];
  hh_show($x); // intersect(intersect(X), Tint)

  /**
   * Container constructors must do the same thing as assignment, otherwise
   * they will end up unifying [2]intersect(X) with Tstring and Tint.
   */
  $_ = Vector {$tuple[0], $tuple[1]};
  $_ = Map {0 => $tuple[0], 1 => $tuple[1]};
  $_ = array($tuple[0], $tuple[1]);
  $_ = array($tuple[0] => 0, $tuple[1] => 1);

  // Now Hack would think that $tuple[0] is an intersection of string and int
  return $tuple[0];
}
