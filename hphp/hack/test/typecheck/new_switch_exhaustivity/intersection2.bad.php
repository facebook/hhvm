<?hh

interface I {}
interface J {}

function main((I & J) $x): void {
  switch ($x) {
    case null:
      break;
  }
}
