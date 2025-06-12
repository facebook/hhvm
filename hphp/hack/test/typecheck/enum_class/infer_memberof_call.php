<?hh

function from_values
  <
    Tk as arraykey,
    Tv
  >(
  Traversable<Tv> $values,
  (function (Tv)[_]: Tk) $key_func,
)[ctx $key_func]: dict<Tk, Tv> {
  return dict[];
}

class C {
}
class D {
}
final class E {
  public function getFieldName(): string {
    return "F";
  }
}

function get1():dict<
  string, HH\MemberOf<C, E>> {
  return dict[];
}
function get2():dict<
  string, HH\MemberOf<D, E>> {
  return dict[];
  }
function isLoggerFieldEnabled(bool $b):void {
  if ($b) {
    $logger_type_fields = get1();
  }
  else {
    $logger_type_fields = get2();
  }
    $logger_field_names = from_values(
      $logger_type_fields,
      $logger_field ==> $logger_field->getFieldName(),
    );
}
