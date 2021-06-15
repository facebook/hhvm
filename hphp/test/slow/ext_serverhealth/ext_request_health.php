<?hh

<<__EntryPoint>>
function main() {
  invariant(HH\jumpstart_source_host() !== null, "");
  var_dump(HH\get_request_health()["jit_maturity"]);
  var_dump(HH\jit_jumpstarted());
  var_dump(HH\jumpstart_prof_tag());
}
