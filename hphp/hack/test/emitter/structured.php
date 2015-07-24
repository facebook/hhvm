<?hh // strict

function tup(): (int, int) {
  $x = 5;
  return tuple(5, $x);
}

function vec(): Vector<int> {
  $x = 2;
  $y = Vector{1, $x, 3};
  $y[] = 10;
  return $y;
}
function imm_vec(): ImmVector<int> {
  $x = 2;
  $y = ImmVector{1, $x, 3};
  return $y;
}
function set(): Set<int> {
  $x = 2;
  $y = Set{1, $x, 3};
  return $y;
}
function imm_set(): ImmSet<int> {
  $x = 2;
  $y = ImmSet{1, $x, 3};
  return $y;
}
function set_string(): Set<string> {
  $x = 'hi';
  $y = Set{'lol', $x, '3'};
  return $y;
}

function map(): Map<string, int> {
  $x = 'hi';
  $y = Map{'lol' => 5, $x => 133 , '3' => 222222};
  return $y;
}
function imm_map(): ImmMap<string, int> {
  $x = 'hi';
  $y = ImmMap{'lol' => 5, $x => 133 , '3' => 222222};
  return $y;
}

function pair(): Pair<string, int> {
  $x = 'hi';
  $y = Pair{$x, 5};
  return $y;
}

class Consts {
  const int A = 0;
  const int B = 1;
}

function test(): void {
  var_dump(tup());

  var_dump(vec());
  var_dump(imm_vec());
  var_dump(set());
  var_dump(imm_set());
  var_dump(set_string());
  var_dump(map());
  var_dump(imm_map());

  var_dump(shape('a' => 'hi', 'b' => 2));
  var_dump(shape('b' => 'hi', 'a' => 2));
  var_dump(shape(Consts::B => 'hi', Consts::A => 2));
  var_dump(shape(Consts::A => 'hi', Consts::B => 2));
}
