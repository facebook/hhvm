<?hh

// Report a parameter result here
function g(dict<string, mixed> $_): void {
}

function f(): void {
  $d = dict['a' => true]; // Report an allocation result here
  g($d); // Don't report a result here due to argument
}
