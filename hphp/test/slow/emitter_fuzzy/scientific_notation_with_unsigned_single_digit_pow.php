<?hh

function __toString(): void {
  $res_str = vec[
    1e3,
    dict[
      ' in ' => 12345,
    ],
  ];
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
