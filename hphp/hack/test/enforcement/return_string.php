<?hh

function foo(): string {
//              ^ enforcement-at-caret
  return "hello";
}
