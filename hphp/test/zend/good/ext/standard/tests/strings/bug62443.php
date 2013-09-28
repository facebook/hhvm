<?php
crypt("foo", '$5$'.chr(0).'abc');
crypt("foo", '$6$'.chr(0).'abc');
echo "OK!";