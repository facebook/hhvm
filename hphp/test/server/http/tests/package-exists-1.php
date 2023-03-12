<?hh

<<__EntryPoint>>
function main() {
  require __DIR__.'/test_multifile.inc';
  runDefaultSandboxTest(1, '-vEval.EnableDecl=1');
}
