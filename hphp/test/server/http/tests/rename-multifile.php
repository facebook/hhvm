<?hh

<<__EntryPoint>>
function main() {
  require __DIR__.'/test_multifile.inc';
  runDefaultSandboxTest(3, '-vEval.EnableDecl=1');
}
