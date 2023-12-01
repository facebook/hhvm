<?hh

function main(/* TAny */ $untyped, /* TAny */ $arr) {
  $x = dict[];
  $x['foo'] = 0;
  $x[null] = 0;
  $x[4.2] = 0;
  $x[$untyped] = 0;
  $x = dict[];
  $x['foo'] = 0;
  $x[null] = 0;
  $x[4.2] = 0;
  $x[$untyped] = 0;
  $x = Map{};
  $x['foo'] = 0;
  $x[null] = 0;
  $x[4.2] = 0;
  $x[$untyped] = 0;
  $x = Vector{};
  $x[0] = 0;
  $x[null] = 0;
  $x[4.2] = 0;
  $x[$untyped] = 0;
  $x = vec[];
  $x[0] = 0;
  $x[null] = 0;
  $x[4.2] = 0;
  $x[$untyped] = 0;
  $arr[0] = 0;
  $arr[null] = 0;
  $arr[4.2] = 0;
  $arr[$untyped] = 0;
}
