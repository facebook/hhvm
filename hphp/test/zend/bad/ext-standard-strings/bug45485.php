<?php

$s =<<< EOD
This text is shown <?XML:NAMESPACE PREFIX = ST1 /><b>This Text disappears</b>
EOD;

$s = strip_tags($s);
echo htmlspecialchars($s),"\n";

$s =<<< EOD
This text is shown <?xml:NAMESPACE PREFIX = ST1 /><b>This Text disappears</b>
EOD;

$s = strip_tags($s);
echo htmlspecialchars($s),"\n";

?>