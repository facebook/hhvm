<?hh // strict

/* HH_FIXME[4110] */
function f(): ?int {
}

function g(): int {
  return f() ?? 0;
}
