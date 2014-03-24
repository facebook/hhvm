<?php
$wddx = <<<WDX
<wddxpacket version="1.0">
<header>
<comment>Content Configuration File</comment>
</header>
<data>
<struct>
<var name="content_queries">
<struct>
<var name="content_113300831086270200">
<struct>
<var name="113301888545229100">
<struct>
<var name="max">
<number>10</number>
</var>
<var name="cache">
<number>4</number>
</var>
<var name="order">
<struct>
<var name="content_113300831086270200">
<struct>
<var name="CMS_BUILD">
<string>desc</string>
</var>
</struct>
</var>
</struct>
</var>
</struct>
</var>
</struct>
</var>
</struct>
</var>
</struct>
</data>
</wddxpacket>
WDX;

var_dump(wddx_deserialize($wddx));
?>
