<?hh

<<__EntryPoint>>
function main() :mixed{
  require __DIR__.'/test_multifile.inc';
  runDefaultSandboxTest(3, '-vEval.EnableDecl=1');
  echo "Enabling OptimizeReifiedParamChecks...\n";
  runDefaultSandboxTest(
    3,
    '-vEval.EnableDecl=1 -vHack.Lang.OptimizeReifiedParamChecks=true',
  );
  echo "Enabling ReuseUnitsByHash...\n";
  runDefaultSandboxTest(
    3,
    '-vEval.EnableDecl=1 -vHack.Lang.OptimizeReifiedParamChecks=true -vEval.ReuseUnitsByHash=1',
  );
}
