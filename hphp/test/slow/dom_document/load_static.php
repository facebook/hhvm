<?php

var_dump(get_class(DOMDocument::loadXML('<html/>')));
var_dump(get_class(DOMDocument::loadHTML('<html/>')));

var_dump(get_class(DOMDocument::loadXML('<html/>', LIBXML_NOWARNING + LIBXML_NOERROR)));
var_dump(get_class(DOMDocument::loadHTML('<html/>', LIBXML_NOWARNING + LIBXML_NOERROR)));
