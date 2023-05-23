<?hh

function exception_as_int(): void {
  (new Exception('not an int')) as int;
}
