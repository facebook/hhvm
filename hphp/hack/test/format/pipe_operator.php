<?hh
function piped(): int {
  // Pipe doesn't force you to format with a newline if you don't want.
  return f() |> g($$) |> h($$);
}
