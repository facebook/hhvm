<?hh
/* dummy include*/

function inc2(): void {
  include(dirname(__FILE__)."/get_included_files_inc3.inc");
  inc3();
}
