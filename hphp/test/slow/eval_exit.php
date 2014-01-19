<?php

print "Before error\n";
eval('blah();');
print "After error, this should not show!\n";
