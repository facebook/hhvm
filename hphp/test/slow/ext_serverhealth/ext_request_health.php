<?hh

<<__EntryPoint>>
function main() {
  assert(hh\jumpstart_source_host() !== null);
  var_dump(hh\get_request_health()["jit_maturity"]);
  var_dump(hh\jit_jumpstarted());
  var_dump(hh\jumpstart_prof_tag());
}
