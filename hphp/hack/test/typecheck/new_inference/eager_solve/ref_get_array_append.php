<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


function vec_append<T>(vec<T> $a, T $x): vec<T> {
  $a[] = $x;
  return $a;
}
function vector_append<T>(Vector<T> $a, T $x): void {
  $a[] = $x;
}

function test_vector_append(?int $i):MyRef<Vector<int>> {
  $references_obj = new MyRef(Vector{});
  if ($i !== null) {
    $references = $references_obj->get();
    $references[] = $i;
    $references_obj->set($references);
  }
  return $references_obj;
}

function test_call_vector_append(?int $i):void {
  $references_obj = new MyRef(Vector{});
  if ($i !== null) {
    $references = $references_obj->get();
    vector_append($references, $i);
    $references_obj->set($references);
  }
}


function test_vec_append(?int $i):MyRef<vec<int>> {
  $references_obj = new MyRef(vec[]);
  if ($i !== null) {
    $references = $references_obj->get();
    $references[] = $i;
    $references_obj->set($references);
    /* Waiting on tyvar variance fix: then we can write MyRef<vec<num>>
       as retutn type
    $references[] = 3.4;
    $references_obj->set($references);
    */
  }
  return $references_obj;
}

function test_call_vec_append(?int $i):void {
  $references_obj = new MyRef(vec[]);
  if ($i !== null) {
    $references = $references_obj->get();
    $references = vec_append($references, $i);
    $references_obj->set($references);
  }
}

final class MyRef<T> {
  public function __construct(public T $value) {}
  public function get(): T {
    return $this->value;
  }
  public function set(T $new_value): void {
    $this->value = $new_value;
  }
}
