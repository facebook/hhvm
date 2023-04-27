<?hh

class C<T> {
  public function non_object_member_read_generic(T $x): void {
    /* HH_FIXME[4062] */
    $x->bar();
  }
}
