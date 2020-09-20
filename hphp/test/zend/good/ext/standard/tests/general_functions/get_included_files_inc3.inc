<?hh
/* dummy include*/

function inc3(): void {
  include(dirname(__FILE__)."/get_included_files_inc1.inc");
  inc1();
}
