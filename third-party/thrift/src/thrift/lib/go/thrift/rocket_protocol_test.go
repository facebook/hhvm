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

package thrift

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestNewRocketException(t *testing.T) {
	kind := ErrorKind_PERMANENT
	blame := ErrorBlame_CLIENT
	safety := ErrorSafety_SAFE
	class := &ErrorClassification{
		Kind:   &kind,
		Blame:  &blame,
		Safety: &safety,
	}
	declaredException := &PayloadExceptionMetadataBase{
		Metadata: &PayloadExceptionMetadata{
			DeclaredException: &PayloadDeclaredExceptionMetadata{
				ErrorClassification: class,
			},
		},
	}
	err := newRocketException(declaredException)
	assert.NotNil(t, err)
	assert.Contains(t, err.Error(), "DeclaredException")
	appUnknownException := &PayloadExceptionMetadataBase{
		Metadata: &PayloadExceptionMetadata{
			AppUnknownException: &PayloadAppUnknownExceptionMetdata{
				ErrorClassification: class,
			},
		},
	}
	err = newRocketException(appUnknownException)
	assert.NotNil(t, err)
	assert.Contains(t, err.Error(), "AppUnknownException")
	appUnknownException2 := &PayloadExceptionMetadataBase{
		Metadata: &PayloadExceptionMetadata{
			AppUnknownException: &PayloadAppUnknownExceptionMetdata{},
		},
	}
	err = newRocketException(appUnknownException2)
	assert.NotNil(t, err)
	assert.Contains(t, err.Error(), "AppUnknownException")
}
