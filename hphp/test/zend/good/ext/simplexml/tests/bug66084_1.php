<?php
echo json_encode(simplexml_load_string('<a><b/><c><x/></c></a>')), "\n";
echo json_encode(simplexml_load_string('<a><b/><d/><c><x/></c></a>')), "\n";
echo json_encode(simplexml_load_string('<a><b/><c><d/><x/></c></a>')), "\n";
echo json_encode(simplexml_load_string('<a><b/><c><d><x/></d></c></a>')), "\n";
?>
