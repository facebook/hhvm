<?php
$s = "het leven is net erwtensoep - je kunt er geen touw aan vastknopen";
for($i=0; $i<3; $i++) {
	echo implode(count_chars($s, $i))."\n";
}
echo $a = count_chars($s, 3), "\n";
echo (int) strlen(count_chars($s, 4)) == 256-strlen($a),"\n";

?>