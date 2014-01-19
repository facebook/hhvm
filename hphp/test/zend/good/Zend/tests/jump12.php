<?php
a: print "ok!\n";
goto c;
declare (ticks=1) {
    b:
        print "ok!\n";
        exit;
}
c:
    print "ok!\n";
    goto b;
?>