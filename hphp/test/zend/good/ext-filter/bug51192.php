<?php
var_dump(filter_var('http://example.com/path', FILTER_VALIDATE_URL));
var_dump(filter_var('http://exa-mple.com/path', FILTER_VALIDATE_URL));
var_dump(filter_var('http://exa_mple.com/path', FILTER_VALIDATE_URL));