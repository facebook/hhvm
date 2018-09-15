<?php


// Based on MediaWiki's ApiFormatWddxTest
// The omitted data in the "b" variable previously caused a fatal error
<<__EntryPoint>>
function main_malformed() {
var_dump(wddx_deserialize(<<<EOT
<wddxPacket version="1.0">
  <header/>
  <data>
    <struct>
      <var name="a">
        <string>foo</string>
      </var>
      <var name="b"/>
    </struct>
  </data>
</wddxPacket>
EOT
));
}
