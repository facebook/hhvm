<?php
// Copyright 2004-present Facebook. All Rights Reserved.
namespace HH\CodeModel {

/*
 *  Returns a string that can be unserialized into an instance of the Code Model
 *  that corresponds to the source code specified in the argument string.
 *
 *  @param @source
 *  A string consisting of PHP source code as would be found in a source file.
 *
 */
<<__Native>>
function get_code_model_for(\string $source) : \string;
}
