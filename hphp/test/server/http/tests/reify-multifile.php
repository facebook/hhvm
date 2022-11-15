<?hh

<<__EntryPoint>>
function main() {
  require __DIR__.'/test_multifile.inc';
  runDefaultSandboxTest(3, '-vEval.EnableDecl=1');

  echo "Enabling ReuseUnitsByHash...\n";
  runDefaultSandboxTest(3, '-vEval.EnableDecl=1 -vEval.ReuseUnitsByHash=1');
}
