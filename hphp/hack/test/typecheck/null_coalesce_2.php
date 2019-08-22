<?hh // strict

/* HH_FIXME[4336] */
function f(): ?int {
}

function g(): int {
  return f() ?? 0;
}
