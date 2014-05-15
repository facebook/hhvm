<?php
$str = "a\xebcd";
echo bin2hex(htmlspecialchars($str, ENT_SUBSTITUTE))."\n";
echo bin2hex(htmlspecialchars(htmlspecialchars($str, ENT_SUBSTITUTE),
                                               ENT_SUBSTITUTE))."\n";
echo bin2hex(htmlspecialchars("b\xeddd", ENT_SUBSTITUTE))."\n";
