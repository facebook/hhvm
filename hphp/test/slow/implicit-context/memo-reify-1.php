<?hh

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_inaccessible<reify T>($x): void {
  var_dump($x is T);
}

<<__EntryPoint>>
function main() :mixed{
  soft_inaccessible<int>(1);
  soft_inaccessible<int>(true);
}
