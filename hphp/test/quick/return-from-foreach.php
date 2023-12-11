<?hh

function f1() :mixed{
  $arr = vec[1,2,3];
  foreach ($arr as $v1) {
    foreach ($arr as $v2) {
      return;
    }
  }
}
<<__EntryPoint>> function main(): void { f1();
echo "Done\n";
}
