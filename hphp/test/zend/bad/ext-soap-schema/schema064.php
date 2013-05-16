<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="dateTime" type="dateTime"/>
			<element name="time" type="time"/>
			<element name="date" type="date"/>
			<element name="gYearMonth" type="gYearMonth"/>
			<element name="gYear" type="gYear"/>
			<element name="gMonthDay" type="gMonthDay"/>
			<element name="gDay" type="gDay"/>
			<element name="gMonth" type="gMonth"/>
		</sequence>
	</complexType>
EOF;
$date = gmmktime(1,2,3,4,5,1976);
putenv('TZ=GMT');
test_schema($schema,'type="tns:testType"',array(
	'dateTime' => $date,
	'time' => $date,
	'date' => $date,
	'gYearMonth' => $date,
	'gYear' => $date,
	'gMonthDay' => $date,
	'gDay' => $date,
	'gMonth' => $date
));
echo "ok";
?>