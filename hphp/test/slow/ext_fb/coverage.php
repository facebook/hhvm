<?hh
<<__EntryPoint>> function main(): void {
fb_enable_code_coverage();

$x
  =
  1
  +
  1
  -
  10
  +
  12
  ;

var_dump(fb_get_code_coverage(false));
var_dump(HH\disable_code_coverage_with_frequency());
}
