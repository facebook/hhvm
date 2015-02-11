<?hh

function array_filter<Tv>(
  Container<Tv> $input,
  ?(function(Tv): bool) $callback = null,
) {}

function test() {
  array_filter(0);
}
