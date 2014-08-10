<?php
class mydt extends datetime
{
	public function __construct($time = 'now', $tz = NULL, $format = NULL)
	{
		if (!empty($tz) && !is_object($tz)) {
			$tz = new DateTimeZone($tz);
		}
		try {
			@parent::__construct($time, $tz);
		} catch (Exception $e) {
			echo "Bad date" . $this->format("Y") . "\n";
		}
	}

};

new mydt("Funktionsansvarig rådgivning och juridik", "UTC");
?>
