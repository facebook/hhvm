<?php

DefineFunction(
  array('name' => 'id',
        'flags' => NoEffect|HipHopSpecific,
        'return' => array(
          'type' => 'Variant',
        ),
        'args' =>
        array(array('name'=> 'v',
                    'type' => 'Variant')),
       ));
