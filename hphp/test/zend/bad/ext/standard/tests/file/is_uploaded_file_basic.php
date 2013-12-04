<?php
// uploaded file
var_dump(is_uploaded_file($_FILES['pics']['tmp_name']));

// not an uploaded file
var_dump(is_uploaded_file($_FILES['pics']['name']));

// not an uploaded file
var_dump(is_uploaded_file('random_filename.txt'));

// not an uploaded file
var_dump(is_uploaded_file('__FILE__'));

// Error cases
var_dump(is_uploaded_file());
var_dump(is_uploaded_file('a', 'b'));

?>