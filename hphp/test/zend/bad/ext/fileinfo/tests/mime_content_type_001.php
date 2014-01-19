<?php

mime_content_type(1);
mime_content_type(NULL);
mime_content_type(new stdclass);
mime_content_type(array());
mime_content_type('foo/inexistent');
mime_content_type('');
mime_content_type("\0");

?>