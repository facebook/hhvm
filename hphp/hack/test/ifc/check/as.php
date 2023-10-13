<?hh

class Data {
  <<__Policied("PUBLIC")>>
  public bool $pub = true;

  <<__Policied("PRIVATE")>>
  public bool $pri = false;
}

class C {}
class D {}

<<__InferFlows>>
function as_exn_leak(Data $data, C $c, D $d): void {
  if ($data->pri) {
    $x = $c;
  } else {
    $x = $d;
  }
  try {
    $x as C; // may throw
  } catch (Exception $_) {
    $data->pub = true;
  }
}

<<__InferFlows>>
function as_null_exn_ok(Data $data, C $c, D $d): void {
  if ($data->pri) {
    $x = $c;
  } else {
    $x = $d;
  }
  try {
    $x ?as C; // no exception is thrown
  } catch (Exception $_) {
    $data->pub = true;
  }
}

<<__InferFlows>>
function as_null_exn_ko(Data $data, C $c, D $d): void {
  if ($data->pri) {
    $x = $c;
  } else {
    $x = $d;
  }
  $data->pub = !($x ?as C);
}
