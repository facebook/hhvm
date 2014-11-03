<?php

// This is a modified version of zend test DOMDocument_saveHTML_error2.php.
// Since parameter type check is performed before detecting invocation of
// non-static method from a static context, the error message differs.

(new DOMDocument())->saveHTML(true);
DOMDocument::saveHTML(new DOMNode());
