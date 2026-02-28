<?hh

class A {
  public darray<string, int> $id_by_event = dict[];
  public darray<string, varray<int>> $ids_by_event = dict[];

  public function fail(?string $event)[]: void {
    if ($event !== null) {
      $this->id_by_event[$event] = 1; // ERROR
      $this->ids_by_event[$event][0] = 1; // ERROR
    }
  }
}

function fail(A $a)[]: void {
  $a->ids_by_event['$event'][0] = 2; // ERROR
}
