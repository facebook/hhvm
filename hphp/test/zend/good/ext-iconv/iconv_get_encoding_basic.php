<?php

iconv_set_encoding("internal_encoding", "UTF-8");
iconv_set_encoding("output_encoding",   "UTF-8");
iconv_set_encoding("input_encoding",    "UTF-8");

var_dump( iconv_get_encoding('internal_encoding') );
var_dump( iconv_get_encoding('output_encoding')   );
var_dump( iconv_get_encoding('input_encoding')    );
var_dump( iconv_get_encoding('all')               );
var_dump( iconv_get_encoding('foo')               );
var_dump( iconv_get_encoding()                    );



iconv_set_encoding("internal_encoding", "ISO-8859-1");
iconv_set_encoding("output_encoding",   "ISO-8859-1");
iconv_set_encoding("input_encoding",    "ISO-8859-1");

var_dump( iconv_get_encoding('internal_encoding') );
var_dump( iconv_get_encoding('output_encoding')   );
var_dump( iconv_get_encoding('input_encoding')    );
var_dump( iconv_get_encoding('all')               );
var_dump( iconv_get_encoding('foo')               );
var_dump( iconv_get_encoding()                    );

?>