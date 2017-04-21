<?hh
function piped(): int {
  return f() |> g($$) |> h($$);
}
