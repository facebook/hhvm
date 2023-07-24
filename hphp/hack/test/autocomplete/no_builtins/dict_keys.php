<?hh

function foo(): string {
}

function bar(dict<string, string> $in): string {
  return $in[AUTO332]; // This should not invoke autocomplete
}
