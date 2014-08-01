<?php
echo basename("y:") . "\n";
echo basename("y:/") . "\n";
echo basename("notdriveletter:file.txt") . "\n";
echo basename("a:\\b:c:d:hello.txt\\hcd:c.txt") . "\n";
echo basename("a:b:c:d:hello.txt\\d:some.txt") . "\n";
echo basename("a:b:c:d:hello\world\a.bmp\c:d:e:f.txt") . "\n";
echo basename("a:\\b:\\c:d:hello\\world\\a.bmp\\d:e:f:g.txt") . "\n";
echo basename("a:\\b:\\c:d:hello/world\\a.bmp\\d:\\e:\\f:g.txt") . "\n";
echo basename("a:\\b:/c:d:hello\\world:somestream") . "\n";
echo basename("a:\\b:\\c:d:hello\\world:some.stream") . "\n";
echo basename("a:/b:\\c:d:hello\\world:some.stream:\$DATA") . "\n";
echo basename("x:y:z:hello\world:my.stream:\$DATA") . "\n";
echo basename("a:\\b:\\c:d:hello\\world:c:\$DATA") . "\n";
echo basename("a:\\b:\\c:d:hello\\d:world:c:\$DATA") . "\n";
?>
==DONE==