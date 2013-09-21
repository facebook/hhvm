<?php 
    chdir(dirname(__FILE__));
    setlocale(LC_ALL, 'en_US.UTF-8');
    dgettext ('foo');
    dgettext ();

    dgettext(array(), 'foo');
    dgettext('foo', array());

?>