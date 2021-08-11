<?hh

function takes_int(int $_): void;

function f(): void {
  $xhp = <hello />; // There is no XHP class named :hello
  takes_int($xhp); // Prevent cascading errors
}
