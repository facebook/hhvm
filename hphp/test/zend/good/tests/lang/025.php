<?hh
function RekTest ($nr) {
    echo " $nr ";
    $j=$nr+1;
    while ($j < 10) {
      echo " a ";
      RekTest($j);
      $j++;
      echo " b $j ";
    }
    echo "\n";
}
<<__EntryPoint>> function main(): void {
RekTest(0);
}
