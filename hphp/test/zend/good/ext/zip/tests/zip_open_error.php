<?php
echo "Test case 1:";
$zip = zip_open("");

echo "Test case 2:";
$zip = zip_open("i_dont_care_about_this_parameter", "this_is_one_to_many");

echo "Test case 3:\n";
$zip = zip_open("/non_exisitng_directory/test_procedural.zip");
echo is_resource($zip) ? "OK" : "Failure";
?>