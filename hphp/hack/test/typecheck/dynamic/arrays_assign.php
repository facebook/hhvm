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

function assign_to_dict(dynamic $d) : void {
  $v = dict[];
  $v[$d] = 1;
  $v["1"];
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
  // should type error
  $d[new C()] = 4;
  // should type error
  $d[new C()] + 4;
}
