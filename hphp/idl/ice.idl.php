<?php
DefinePreamble(<<<CPP

CPP
);

BeginClass(
	array(
		"name"   => "ICE",
		"desc"   => "create a class of ICE",
		"flags"  => HasDocComment,
		"footer" => "",
	)
);

DefineFunction(
	array(
		'name'   => "__construct",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => null,
		),
	)
);

DefineFunction(
	array(
		'name'   => "stringToProxy",
		'desc'   => "create a proxy to server",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => Object,
			'desc'  => "return a ObjectPtr proxy.",
		),
		'args'   => array(
			array(
			'name'  => "str",
			'type'  => String,
			'desc'  => "set a proxy between server and client",
			),
		),
	)
);

EndClass();

DefineFunction(
	array(
		'name'   => "ice_initialize",
		'desc'   => "ice initialize",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => Object,
			'desc'  => "return a object of ice",
		),
	)
);

BeginClass(
	array(
		"name"   => "Proxy",
		"desc"   => "a Proxy object",
		"flags"  => HasDocComment,
	)
);

DefineFunction(
	array(
		'name'   => "__construct",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => null,
		),
	)
);

DefineFunction(
	array(
		'name'   => "__call",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => Variant,
		),
		'args'   => array(
			array(
			'name' => "name",
			'type' => Variant, 
			),
			array(
			'name' => "args",
			'type' => Variant,
			),
		),
	)
);

DefineFunction(
	array(
		'name'	  => "ice_oneway",
		'desc'	  => "set the transform is single direct",
		'flags'   => HasDocComment,
		'return'  => array(
			'type'  => Object,
			'desc'  => "return a proxyObject",
		),
	)
);

DefineFunction(
	array(  
		'name'    => "ice_datagram",
		'desc'    => "set the transform is datagram direct",
		'flags'   => HasDocComment,
		'return'  => array(
			'type'  => Object,
			'desc'  => "return a proxyObject",
		),
		)
);

DefineFunction(
	array(
		'name'    => "ice_isTwoway",
		'desc'    => "set the transform is bothway",
		'flags'   => HasDocComment,
		'return'  => array(
			'type'  => Object,
			'desc'  => "return a bothway reslut is or not success",
		),
	)
);

DefineFunction(
	array(
		'name'    => "ice_secure",
		'desc'    => "set the proxyObject's secure whether open",
		'flags'   => HasDocComment,
		'return'  => array(
			'type'  => Object,
			'desc'  => "return a proxyObject with secure",
		),
		'args'    => array(
			array(
			'name'  => "secure",
			'type'  => Boolean,
			'value' => "false",
			'desc'  => "set the secure whether open",
			),
		),
	)
);


DefineFunction(
	array(
		'name'   => "ice_timeout",
		'desc'   => "set for proxy a timeout",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => Object,
			'desc'  => "return the proxy of the last",
		),
		'args'   => array(
			array(
			'name'  => "lasttime",
			'type'  => Int64,
			'value' => "null",
			'desc'  => "set the outtime",
			),
		),
	)
);

DefineFunction(
	array(
		'name'   => "ice_context",
		'desc'   => "get ice context",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  => Object,
			'desc'  => "return the object of proxy",
		),
		'args'   => array(
			array(
			'name'  => "ctx",
			'type'  => StringMap,
			'value' => "null",
			'desc'  => "set the paramter of people want,the kind is key value",
			),
		),
	)
);

DefineFunction(
				array(
						'name'   => "ice_checkedCast",
						'desc'   => "ice_checkedCast",
						'flags'  => HasDocComment,
						'return' => array(
								'type'  => Object,
								'desc'  => "return the object of proxy",
								),
						'args'   => array(
								array(
										'name'  => "classid",
										'type'  => String,
										'desc'  => "",
									 ),
								array(
										'name'  => "facetOrCtx",
										'type'  => Variant,
										'value' => "null",
										'desc'  => "", 
									 ),  
								array(
										'name'  => "ctx",
										'type'  => Variant,
										'value' => "null",
										'desc'  => "", 
									 ),  

								),  
					)   
				);
//
DefineFunction(
		array(
			'name'   => "ice_uncheckedcast",
			'desc'   => "ice_uncheckedCast",
			'flags'  => HasDocComment,
			'return' => array(
					'type'  => Object,
					'desc'  => "return the object of proxy",
			),
			'args'   => array(
					array(
						'name'  => "classid",
						'type'  => String,
						'desc'  => "",
						 ),
					array(
						'name'  => "facet",
						'type'  => Variant,
						'value' => "null",
						'desc'  => "",
						 ),
			),
				)
				);

EndClass();

DefineFunction(
	array(
		'name'  => "icephp_defineproxy",
		'desc'  => "IcePHP_defineProxy",
		'flags' => HasDocComment,
		'return'=> array(
			'type' => Object,
			'desc' => "return the object of proxy",
		),
		'args'  => array(
			array(
				'name'  => "classobj",
				'type'  => Object,
			),
		),
	)
);


DefineFunction(
	array(
		'name'   => "icephp_declareclass",
		'desc'   => "IcePHP_declareClass",
		'flags'  => HasDocComment,
		'return' => array(
			'type' => Object,
			'desc' => "",
		),
		'args'   => array(
			array(
				'name' => "id",
				'type' => String,
				'value'=> "null", 
			),
		),
	)
);


DefineFunction(
	array(
		'name'  => "icephp_defineexception",
		'desc'  => "IcePHP_defineException",
		'flags' => HasDocComment,
		'return'=> array(
			'type'  => Object,
			'desc'  => "throw the exception",
		),
		'args'  => array(
			array(
				'name' => "id",
				'type' => String,
				'value' => "null",
			),
			array(
				'name' => "name",
				'type' => String,
				'value' => "null",
			),
			array(
				'name' => "base",
				'type' => Variant,
				'value' => "null",
			),
			array(
				'name' => "members",
				'type' => Variant,
				'value' => "null",
			),
		),
	)
);



DefineFunction(
	array(
		'name'  => "create_typeinfobyid",
		'desc'  => "creat const global var type",
		'flags' => HasDocComment,
		'return'=> array(
			'type'  => Variant,
			'desc'  => "",
		),
		'args'  => array(
			array(
				'name' => "id",
				'type' => Int32,
				'value' => "0",
			),
		),
	)
);


DefineFunction(
	array(
		'name'  => "icephp_stringifyexception",
		'desc'  => "icephp_stringifyexception",
		'flags' => HasDocComment,
		'return'=> array(
			'type'  => String,
			'desc'  => "throw the string vertify exception",
		),
		'args'  => array(
			array(
				'name' => "value",
				'type' => Object,
				'value' => "null",
			),
			array(
				'name' => "target",
				'type' => Object,
				'value' => "null",
			),
		),
	)
);


DefineFunction(
	array(
		'name'  => "icephp_stringify",
		'desc'  => "icephp_stringify",
		'flags' => HasDocComment,
		'return'=> array(
			'type' => String,
			'desc' => "vertify for the string",
		),
		'args' => array(
			array(
				'name' => "value",
				'type' => Object,
				'value' => "null",
			),
			array(
				'name' => "target",
				'type' => Object,
				'value' => "null",
			),
		),
	)
);


DefineFunction(
	array(
		'name'   => "icephp_definesequence",
		'desc'   => "IcePHP_defineSequence",
		'flags'  => HasDocComment,
		'return' => array(
			'type'  =>  Object,
			'desc'  => "return the kind of sequence",
		),
		'args'   => array(
			array(
				'name'  => "id",
				'type'  => String,
				'value' => "null",
			),
			array(
				'name'  => "elementtype",
				'type'  => Variant,
				'value' => "null",
			),
		),
	)
);


//
DefineFunction(
	array(
		'name'   => "icephp_defineclass",
		'desc'   => "IcePHP_defineClass",
		'flags'  => HasDocComment,
		'return' => array(
			'type' => Object,
			'desc' => "return the object of dynamic class",
		),
		'args'   => array(
			array(
				'name'  => "id",
				'type'  => String,
			),
			array(
				'name'  => "name",
				'type'  => String,
			),
			array(
				'name'  => "isabstract",
				'type'  => Boolean,
				'value' => "true",
			),
			array(
				'name'  => "base",
				'type'  => Object,
				'value' => "null",
			),
			array(
				'name'  => "interfaces",
				'type'  => Variant,
				'value' => "null",
			),
			array(
				'name'  => "members",
				'type'  => Variant,
				'value' => "null",
			),
		),
	)
);
//
//
DefineFunction(
				array(
						'name'   => "icephp_defineoperation",
						'desc'   => "IcePHP_defineOperation",
						'flags'  => HasDocComment,
						'return' => array(
								'type' => null,
								'desc' => "return the object of dynamic class",
								),  
						'args'   => array(
								array(
										'name'  => "classobj",
										'type'  => Object,
										'value' => "null",
									 ),  
								array(
										'name'  => "funname",
										'type'  => String,
										'value' => "null",
									 ),  
								array(
										'name'  => "mode",
										'type'  => Int32,
										'value' => "null",
									 ),  
								array(
										'name'  => "sendmode",
										'type'  => Int32,
										'value' => "null",
									 ),  
								array(
										'name'  => "inparams",
										'type'  => Variant,
										'value' => "null",
									 ),  
								array(
										'name'  => "outparams",
										'type'  => Variant,
										'value' => "null",
									 ),
								array(
										'name'  => "returntype",
										'type'  => Variant,
										'value' => "null",
									),
								array(
										'name'  => "exceptions",
										'type'  => Variant,
										'value' => "null",
									),
								),
					)
				);


BeginClass(
								array(
										"name"   => "IcePHP_class",
										"desc"   => "a null class for proxy",
										"flags"  => HasDocComment,
									 )   
);

DefineFunction(
								array(
										'name'   => "__construct",
										'flags'  => HasDocComment,
										'return' => array(
												'type'  => null,
												),
									 )
);

EndClass();

//
BeginClass(
				array(
						"name"   => "TypeInfo",
						'bases'  => array('UnmarshalCallback'),
						"desc"   => "type for member",
						"flags"  => HasDocComment,
					 )   
		  );

DefineFunction(
				array(
						'name'   => "__construct",
						'flags'  => HasDocComment,
						'return' => array(
								'type'  => null,
								),  
					 )   
			  );


EndClass();


/////////////////////////////////////////////////////////////////
DefineFunction(
				array(
						'name'   => "ice_find",
						'flags'  => HasDocComment,
						'return' => array(
								'type'  => Object,
								),  
						'args'   => array(
							array(
								'name'  => "ice_name",
								'type'  => String,
								'value' => "null",
								'desc'  => "return a ice object pass the paramter of ice_name",
							),
						),
					 )   
			  );




DefineFunction(
				array(
						'name'   => "ice_register",
						'flags'  => HasDocComment,
						'return' => array(
								'type'  => Boolean,
								),  
						'args'   => array(
								array(
										'name'  => "ice_object",
										'type'  => Object,
										'desc'  => "",
								),
								array(
										'name'  => "ice_name",
										'type'  => String,
										'value' => "null",
										'desc'  => "return a ice object pass the paramter of ice_name",
									 ),
								array(
										'name'  => "expires",
										'type'  => Int64,
										'value' => "null",
								),
						),
					 )
			  );