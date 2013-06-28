<?php
abstract class Ice_Exception extends Exception
{
		public function __construct($message = '')
		{
				parent::__construct($message);
		}

		abstract public function ice_name();
}

abstract class Ice_UserException extends Ice_Exception
{
		public function __construct($message = '')
		{
				parent::__construct($message);
		}
}

abstract class Ice_LocalException extends Ice_Exception
{
		public function __construct($message = '')
		{
				parent::__construct($message);
		}
}

interface Ice_Object
{
		public function ice_isA($id);
		public function ice_ping();
		public function ice_ids();
		public function ice_id();

		//
		// No need to define these here; the marshaling code will invoke them if defined by a subclass.
		//
		//public function ice_preMarshal();
		//public function ice_postUnmarshal();
}

abstract class Ice_ObjectImpl implements Ice_Object
{
		public function ice_isA($id)
		{
				return array_search($id, ice_ids());
		}

		public function ice_ping()
		{
		}

		public function ice_ids()
		{
				return array(ice_id());
		}

		public function ice_id()
		{
				return "::Ice::Object";
		}
}

$IcePHP__t_string=create_typeinfobyid(0);
$IcePHP__t_long=create_typeinfobyid(1);
$IcePHP__t_int=create_typeinfobyid(2);
$Ice__t_Object=create_typeinfobyid(3);
$IcePHP__t_double=create_typeinfobyid(4);
?>
