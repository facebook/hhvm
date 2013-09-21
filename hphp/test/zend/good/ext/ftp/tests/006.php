<?php
$ftp=null;

var_dump(ftp_connect(array()));
var_dump(ftp_connect('127.0.0.1', 0, -3));
var_dump(ftp_raw($ftp));
var_dump(ftp_mkdir($ftp));
var_dump(ftp_rmdir($ftp));
var_dump(ftp_nlist($ftp));
var_dump(ftp_rawlist($ftp));
var_dump(ftp_fget($ftp));
var_dump(ftp_nb_fget($ftp));
var_dump(ftp_nb_get($ftp));
var_dump(ftp_pasv($ftp));
var_dump(ftp_nb_continue());
var_dump(ftp_fput());
var_dump(ftp_nb_fput($ftp));
var_dump(ftp_put($ftp));
var_dump(ftp_nb_put($ftp));
var_dump(ftp_size($ftp));
var_dump(ftp_mdtm($ftp));
var_dump(ftp_rename($ftp));
var_dump(ftp_site($ftp));
var_dump(ftp_set_option($ftp));
var_dump(ftp_get_option($ftp));

?>