<?hh

<<__SupportDynamicType>>
class B<<<__NoRequireDynamic>> T> {
}

function f(B<int> $i, B<int> $j) : void {
}

class C {
  public function f(B<int> $i) : void {
    $x = $i as dynamic;
    $x->m();
    $i->m();
    /* Since B<int> isn't enforceable, these work because of the B<int>
     * part of the union and not the dynamic part */
    f($i, $x);
  }
}
