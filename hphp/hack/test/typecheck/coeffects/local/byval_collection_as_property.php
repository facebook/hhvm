<?hh

class A {
  public darray<string, int> $id_by_event = darray[];
  public darray<string, varray<int>> $ids_by_event = darray[];

  <<__Pure, __Mutable>>
  public function mutableMethodOk(?string $event)[]: void {
    if ($event !== null) {
      $this->id_by_event[$event] = 1; // OK
      $this->ids_by_event[$event][0] = 1; // OK
    }
  }

  <<__Pure>>
  public function fail(?string $event)[]: void {
    if ($event !== null) {
      $this->id_by_event[$event] = 1; // ERROR
      $this->ids_by_event[$event][0] = 1; // ERROR
    }
  }
}

<<__Pure>>
function mutable_param(<<__Mutable>> A $a)[]: void {
  $a->ids_by_event['$event'][0] = 2; // OK
}

<<__Pure>>
function fail(A $a)[]: void {
  $a->ids_by_event['$event'][0] = 2; // ERROR
}
