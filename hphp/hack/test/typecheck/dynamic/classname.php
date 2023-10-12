<?hh // strict

function testClassName(classname<dynamic> $x): void {
  $y = $x::staticMeth(); // $y : dynamic
  hh_show($y);
  $y = $x::staticProp; // $y : dynamic
  hh_show($y);
}
