<?php
error_reporting(E_ALL);

session_start();

# Do not remove \r from this tests, they are essential!
?>
<html>
  <head>
    <title>Bug #36459 Incorrect adding PHPSESSID to links, which contains \r\n</title>
  </head>
  <body>
    <p>See source html code</p>
    <a href="/b2w/www/ru/adm/pages/?action=prev&rec_id=8&pid=2"
       style="font: normal 11pt Times New Roman">incorrect link</a><br />
    <br />
    <a href="/b2w/www/ru/adm/pages/?action=prev&rec_id=8&pid=2" style="font: normal 11pt Times New Roman">correct link</a>
  </body>
</html>
