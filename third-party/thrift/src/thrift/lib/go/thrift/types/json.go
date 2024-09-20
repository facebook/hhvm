/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package types

var (
	JSON_COMMA                   = []byte{','}
	JSON_COLON                   = []byte{':'}
	JSON_LBRACE                  = []byte{'{'}
	JSON_RBRACE                  = []byte{'}'}
	JSON_LBRACKET                = []byte{'['}
	JSON_RBRACKET                = []byte{']'}
	JSON_QUOTE                   = byte('"')
	JSON_QUOTE_BYTES             = []byte{'"'}
	JSON_NULL                    = []byte{'n', 'u', 'l', 'l'}
	JSON_TRUE                    = []byte{'t', 'r', 'u', 'e'}
	JSON_FALSE                   = []byte{'f', 'a', 'l', 's', 'e'}
	JSON_INFINITY                = "Infinity"
	JSON_NEGATIVE_INFINITY       = "-Infinity"
	JSON_NAN                     = "NaN"
	JSON_INFINITY_BYTES          = []byte{'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NEGATIVE_INFINITY_BYTES = []byte{'-', 'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NAN_BYTES               = []byte{'N', 'a', 'N'}
	json_nonbase_map_elem_bytes  = []byte{']', ',', '['}
)
