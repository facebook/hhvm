<?hh

function assign_to_vector(dynamic $d) : void {
  $v = Vector{};
  $v[$d] = 1;
}

function assign_to_vec(dynamic $d) : void {
  $v = vec[];
  $v[$d] = 1;
}

function assign_to_map_int(dynamic $d) : void {
  // should type error
  $v = Map<int, _>{};
  $v[$d] = 1;
}

function expect_map_int(Map<int,int> $i) : void {}

function assign_to_map(dynamic $d) : void {
  $v = Map{};
  $v[$d] = 1;
  // should type error
  expect_map_int($v);
}

function expect_map_ak(Map<arraykey,int> $i) : void {}

function assign_to_map2(dynamic $d) : void {
  $v = Map{};
  // shouldn't error because the Map can be given type
  // Map<arraykey, int>; however, inference is incomplete
  // and needs to know the key type is arraykey to work
  // out the enforcement of the key type ahead of time.
  // So this does have an error, but PartialEnforcement
  // with sound dynamic will fix this.
  $v[$d] = 1;
  expect_map_ak($v);
}

function assign_to_map3(dynamic $d) : void {
  $v = Map<arraykey, int>{};
  $v[$d] = 1;
  expect_map_ak($v);
}

function assign_to_dict(dynamic $d) : void {
  $v = dict[];
  $v[$d] = 1;
}

function assign_to_dict_int(dynamic $d) : void {
  $v = dict<int, _>[];
  $v[$d] = 1;
}

function assign_to_string(dynamic $d) : void {
  $v = "123";
  $v[$d] = "1";
  $s = $v[$d];
}

class C {}
function assign_to_dynamic(dynamic $d) : void {
  // should type error, since hhvm will enforce that the key type is arraykey
  // But this is not currently checked
  $d[new C()] = 4;
  // should type error, since hhvm will enforce that the key type is arraykey
  // But this is not currently checked
  $d[new C()] + 4;
}
