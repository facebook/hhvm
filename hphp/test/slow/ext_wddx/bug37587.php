<?php

var_dump(wddx_deserialize(<<<EOF
<wddxPacket version='1.0'>
<header/>
<data>
  <array length='1'>
    <var>
      <struct>
        <var name='test'><string>Hello World</string></var>
      </struct>
    </var>
  </array>
</data>
</wddxPacket>
EOF
));

?>
===DONE===
