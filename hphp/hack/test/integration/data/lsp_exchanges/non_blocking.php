<?hh // strict

function non_blocking_definition(): int {
  return 4;
}

function non_blocking(): int {
  return non_blocking_definition();
}
