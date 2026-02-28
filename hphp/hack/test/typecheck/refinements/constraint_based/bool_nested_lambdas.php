<?hh

function f(vec<string> $encoded_data): vec<bool> {
  return \HH\Lib\Vec\map($encoded_data, $encoded_line ==> $encoded_line)
    |> \HH\Lib\Vec\map($$, $v ==> {
      if ($v is bool) {
        return $v;
      } else if ($v === "1" || $v === "true") {
        return true;
      } else {
        return false;
      }
    });
}
