<?php

var_dump(UConverter::transcode("This is an ascii string", 'utf-8', 'latin1'));
var_dump(urlencode(UConverter::transcode("Espa\xF1ol", 'utf-8', 'latin1')));
var_dump(urlencode(UConverter::transcode("Stra\xDFa",  'utf-8', 'latin1')));
var_dump(bin2hex(UConverter::transcode("\xE4", 'utf-8', 'koi8-r')));
