<?hh

function f(): void {
  $key = 'key';
  dict[$key => 42];  // No shape-like dicts are reported here
  dict['key' => 42]; // A shape-like dict is reported here
}
