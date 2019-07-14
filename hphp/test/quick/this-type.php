<?hh

function a() {
  return function() { return $this->b; };
}
<<__EntryPoint>> function main(): void {
$c = a();
$c();
}
