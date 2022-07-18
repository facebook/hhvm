<?hh

function f(): void {
  $d = dict['a' => true]; // Report an allocation result here
  inspect($d); // Don't report a result here due to argument
}
