<?hh

<<__EntryPoint>>
function main_bcscale() :mixed{
var_dump(bcdiv("105", "6.55957", 3));
bcscale(3);
var_dump(bcdiv("105", "6.55957"));
bcscale(0);
}
