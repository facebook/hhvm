<?hh

type Tvec = vec<int>;
type Tdict = dict<arraykey, string>;
type Tkeyset = keyset<int>;

function is_tvec(mixed $x): void {
  if ($x is Tvec) {
    echo "vec\n";
  } else {
    echo "not vec\n";
  }
}

function is_tdict(mixed $x): void {
  if ($x is Tdict) {
    echo "dict\n";
  } else {
    echo "not dict\n";
  }
}

function is_tkeyset(mixed $x): void {
  if ($x is Tkeyset) {
    echo "keyset\n";
  } else {
    echo "not keyset\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_array() :mixed{
is_tvec(vec['foo']);
is_tvec(vec[1]);
is_tvec(dict[]);
is_tvec(dict['a' => 'b']);
is_tvec(keyset[]);
is_tvec(keyset['foo']);

echo "\n";

is_tdict(vec['foo']);
is_tdict(vec[1]);
is_tdict(dict[]);
is_tdict(dict['a' => 'b']);
is_tdict(keyset[]);
is_tdict(keyset['foo']);

echo "\n";

is_tkeyset(vec['foo']);
is_tkeyset(vec[1]);
is_tkeyset(dict[]);
is_tkeyset(dict['a' => 'b']);
is_tkeyset(keyset[]);
is_tkeyset(keyset['foo']);
}
