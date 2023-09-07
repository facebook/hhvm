<?hh

class C {}

function nullable_class(?C $x): void {
  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

function nullable_class_not_null1(?C $x): void {
  if ($x is null) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

function nullable_class_not_null2(?C $x): void {
  switch ($x) {
    case null:
      return;
    default:
      break;
  }

  switch ($x) {
    // Because of control flow, this is a redundant case
    // However, type inference misses this
    // Hence still a safe overapproximation
    case null:
      return;
    default:
      return;
  }
}

function nullable_class_not_class(?C $x): void {
  if ($x is C) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}
