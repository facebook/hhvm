<?php
var_dump($_FILES);
var_dump($_POST);
if (is_uploaded_file($_FILES["file1"]["tmp_name"])) {
	var_dump(file_get_contents($_FILES["file1"]["tmp_name"]));
}
?>