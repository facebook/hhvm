<?hh

<<__EntryPoint>>
function main() {
 a:
  print "ok!\n";
  goto c;
 b:
  print "ok!\n";
  exit;
 c:
  print "ok!\n";
  goto b;
}
