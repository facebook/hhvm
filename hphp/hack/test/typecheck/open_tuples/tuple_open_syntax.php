<?hh



// Legal tuple types
type T1 = (int,optional string);
type T2 = (int, mixed...);
type T3 = (optional bool, arraykey...);
type T4 = (bool,int,optional string,optional float);
type T5 = (int);
type T6 = (optional bool);
type T7 = (string...);

// These types will be expanded in decling
function test1(T1 $x1, T2 $x2, T3 $x3, T4 $x4, T5 $x5, T6 $x6, T7 $x7):void {
  hh_show($x1);
  hh_show($x2);
  hh_show($x3);
  hh_show($x4);
  hh_show($x5);
  hh_show($x6);
  hh_show($x7);
}
// These types will be parsed directly
function other((int, optional string) $x1, (int, mixed...) $x2, (optional bool, arraykey...) $x3, (bool,int,optional string,optional float) $x4, (int) $x5, (optional bool) $x6, (string...) $x7):void {
  hh_show($x1);
  hh_show($x2);
  hh_show($x3);
  hh_show($x4);
  hh_show($x5);
  hh_show($x6);
  hh_show($x7);
}
