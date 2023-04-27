<?hh

<<__EntryPoint>>
function main(): void {
  $z = 100 +
    () ==>{
    $x = 3;
    // expect variable extracted to here
    return /*range-start*/300
      //
      +
      2/*range-end*/;
}
}
