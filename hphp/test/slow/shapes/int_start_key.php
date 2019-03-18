<?hh

function f(): shape(
  "123start" => int
) {
  return shape(
    "123start" => 4
  );
}

<<__EntryPoint>>
function main(): void {
  $x = f();
  echo $x["123start"];
}
