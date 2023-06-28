<?hh

function switchtest ($i, $j)
:mixed{
    switch ($i) {
        case 0:
                switch($j) {
                    case 0:
                        echo "zero";
                        break;
                    case 1:
                        echo "one";
                        break;
                    default:
                        echo $j;
                        break;
                }
                echo "\n";
                break;
        default:
                echo "Default taken\n";
    }
}
<<__EntryPoint>> function main(): void {
for ($i=0; $i<3; $i++) {
  for ($k=0; $k<10; $k++) {
    switchtest (0,$k);
  }
}
}
