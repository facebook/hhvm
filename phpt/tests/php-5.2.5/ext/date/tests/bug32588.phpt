--TEST--
Bug #32588 (strtotime() error for 'last xxx' DST problem)
--FILE--
<?php
///putenv("TZ=America/New_York");

echo date('D Y/m/d/H:i:s', strtotime('last saturday', 1112703348)). "\n";
echo date('D Y/m/d/H:i:s', strtotime("last sunday", 1112703348)). "\n";
echo date('D Y/m/d/H:i:s', strtotime('last monday', 1112703348)). "\n";
