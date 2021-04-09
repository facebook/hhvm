<?hh

function format($dt) {
  var_dump(date_format($dt, "Y-m-d H:i:s"));
}


<<__EntryPoint>>
function main_date() {

$tmp = "cr";
date_default_timezone_set('UTC');

for($a = 0;$a < strlen($tmp); $a++){
    echo $tmp[$a], ': ', date($tmp[$a], 1043324459)."\n";
}

date_default_timezone_set("MET");

for($a = 0;$a < strlen($tmp); $a++){
    echo $tmp[$a], ': ', date($tmp[$a], 1043324459)."\n";
}
}
