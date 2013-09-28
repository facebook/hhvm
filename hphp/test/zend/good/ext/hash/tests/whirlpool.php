<?php
echo hash('whirlpool', ''), "\n";
echo hash('whirlpool', $s='---qwertzuiopasdfghjklyxcvbnm------qwertzuiopasdfghjklyxcvbnm---'), "\n";
echo hash('whirlpool', str_repeat($s.'0', 1000)), "\n";
?>