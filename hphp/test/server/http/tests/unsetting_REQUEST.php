<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  requestAll(
    vec['test_unsetting_REQUEST.php?Key1=Value1&Key2=Value2'],
  );
  requestAll(
    vec['test_unsetting_REQUEST.php?Key1=Value1&Key2=Value2'],
    '-vEval.DisableRequestSuperglobal=1'
  );
}
