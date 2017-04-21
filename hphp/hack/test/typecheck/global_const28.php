<?hh

const MUST_PREPARE = /* UNSAFE_EXPR */ "lol";

function f(): bool {
  return MUST_PREPARE;
}
