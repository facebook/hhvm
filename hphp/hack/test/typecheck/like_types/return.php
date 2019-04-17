<?hh

function dynamic_int(): dynamic {
  return 'string'; // ok
}

function like_int(): ~int {
  return 'string'; // error TODO(T42626544) implement coercion behavior
}

function enforced_int1(): int {
  return 'string' as dynamic; // ok TODO(T42626544) implement coercion behavior
}

function enforced_int2(): int {
  return like_int(); // ok TODO(T42626544) implement coercion behavior
}
