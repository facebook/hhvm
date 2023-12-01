<?hh

function ok_value_collection(): void {
  keyset<int>[]; // OK
  keyset<arraykey>[]; // OK
  Set<int> {}; // OK
  Vector<int> {}; // OK
}

function ok_keyed_value_collection(): void {
  dict<int, int>[]; // OK
  dict<arraykey, int>[]; // OK
  Map<arraykey, int> {}; // OK
}

function ko_value_collection(): void {
  keyset<bool>[]; // KO: bool </: arraykey
}

function ko_keyed_value_collection(): void {
  dict<bool, int>[]; // KO: bool </: arraykey
}

function ko_darray(): void {
  dict<bool, int>[]; // KO: bool </: arraykey
}

function ko_map(): void {
  Map<bool, int> {}; // KO: bool </: arraykey
}
