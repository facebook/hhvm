<?php

var_dump(dcngettext(1,1,1,1));
var_dump(dcngettext(1,1,1,1,1));
var_dump(dcngettext("test","test","test",1,1));
var_dump(dcngettext("test","test","test",0,0));
var_dump(dcngettext("test","test","test",-1,-1));
var_dump(dcngettext("","","",1,1));
var_dump(dcngettext("","","",0,0));

echo "Done\n";
?>