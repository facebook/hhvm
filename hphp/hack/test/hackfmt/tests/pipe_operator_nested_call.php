<?hh
function piped(): int {
  $a = Foo::aaaaaaaaaaaaaaaaaaaaa()
    |> bar(Bar::bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb()->g($$))
    |> h($$);
}
