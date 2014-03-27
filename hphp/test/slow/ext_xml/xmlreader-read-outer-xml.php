<?php
$xml = <<<EOF
<a>
<b>0</b>
<c>1<d>2<e>3</e>4</d>5</c>
</a>
EOF;
$reader = new XMLReader;
$reader->XML($xml);
while($reader->read()) {
  var_dump($reader->readOuterXML());
}
