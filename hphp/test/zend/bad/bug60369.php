<?php

trait PropertiesTrait {
   static $same = true;
}

class Properties {
   use PropertiesTrait;
   public $same = true;
}

?>