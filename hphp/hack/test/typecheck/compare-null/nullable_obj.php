<?hh //strict

interface Bar {}

function get(?Bar $arg): void {
  if ($arg === null) {
  }
}
