<?php

function checkForClosedFilePointer($host, $curl_option, $description) {
	$fp = fopen(dirname(__FILE__) . '/bug54798.tmp', 'w+');

	$ch = curl_init();

	// we also need CURLOPT_VERBOSE to be set to test CURLOPT_STDERR properly
	if (CURLOPT_STDERR == $curl_option) {
		curl_setopt($ch, CURLOPT_VERBOSE, 1);
	}

    if (CURLOPT_INFILE == $curl_option) {
        curl_setopt($ch, CURLOPT_UPLOAD, 1);
    }

	curl_setopt($ch, $curl_option, $fp);
	
	curl_setopt($ch, CURLOPT_URL, $host);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

	fclose($fp); // <-- premature close of $fp caused a crash!

	curl_exec($ch);

	curl_close($ch);

	echo "Ok for $description\n";
}

$options_to_check = array(
	"CURLOPT_STDERR",
    "CURLOPT_WRITEHEADER",
    "CURLOPT_FILE",
    "CURLOPT_INFILE"
);

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
foreach($options_to_check as $option) {
	checkForClosedFilePointer($host, constant($option), $option);
}

?>
<?php @unlink(dirname(__FILE__) . '/bug54798.tmp'); ?>