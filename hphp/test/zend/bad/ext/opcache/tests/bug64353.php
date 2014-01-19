<?php
class BugLoader extends php_user_filter {
	public function filter($in, $out, &$consumed, $closing) {
		if (!class_exists("Test")) {
			eval("class Test extends ArrayObject {}");
		}
		while ($bucket = stream_bucket_make_writeable($in)) {
			$consumed += $bucket->datalen;
			stream_bucket_append($out, $bucket);
		}
		return PSFS_PASS_ON;
	}
}

stream_filter_register('bug.test', 'BugLoader');
include "php://filter/read=bug.test/resource=data://text/plain,<?php\n";
echo "OK\n";
?>