<?php
ini_set("session.serialize_handler","wddx");
session_start();
$str = "<wddxPacket version='1.0'><header/><data><struct>
  <var name='login_ok'><boolean value='true'/></var>
  <var name='name'><string>somename</string></var>
  <var name='integer'><number>34</number></var>
  </struct></data></wddxPacket>";
session_decode($str);

$data = $_SESSION;

var_dump($data);
