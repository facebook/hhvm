<?php

print "Before silenced error\n";
@blah();
print "After silenced error, this should not show!\n";
