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

import (
	"fmt"
	"reflect"
	"strings"
)

// StructToString returns a string representation of the struct.
func StructToString(value reflect.Value) string {
	// Ensure that the input is a struct pointer (to prevent reflect panics downstream).
	if value.Type().Kind() != reflect.Pointer || value.Type().Elem().Kind() != reflect.Struct {
		return "<invalid>"
	}

	// !!! NOTE: Go may invoke String() method on a nil pointer,
	// so an explicit 'nil' check is required to ensure safety.
	if value.IsNil() {
		return "<nil>"
	}

	concreteStructValue := value.Elem()
	concreteStructType := concreteStructValue.Type()

	var sb strings.Builder

	sb.WriteString(concreteStructType.Name() + "({")

	for i := 0; i < concreteStructType.NumField(); i++ {
		fieldInfo := concreteStructType.Field(i)
		sb.WriteString(fieldInfo.Name + ":")

		fieldValue := concreteStructValue.Field(i)
		printedValue := fieldValue
		// If this field is a non-struct pointer and not 'nil'...
		if fieldValue.Type().Kind() == reflect.Ptr && fieldValue.Type().Elem().Kind() != reflect.Struct && !fieldValue.IsNil() {
			// Dereference the pointer.
			printedValue = fieldValue.Elem()
		}
		sb.WriteString(fmt.Sprintf("%v", printedValue.Interface()))

		if i < concreteStructType.NumField()-1 {
			sb.WriteString(" ")
		}
	}

	sb.WriteString("})")

	return sb.String()
}
