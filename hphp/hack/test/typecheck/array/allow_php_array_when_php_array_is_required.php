<?hh // partial

function consumesUnknownArray(array $array): void {}

function providesUnknownArray(): array {
  return array("tingley" => 0);
}

function test(): void {
  consumesUnknownArray(providesUnknownArray());
}
