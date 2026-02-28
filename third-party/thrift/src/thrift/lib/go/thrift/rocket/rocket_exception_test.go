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

package rocket

import (
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/stretchr/testify/require"
)

func TestNewRocketException(t *testing.T) {
	class := &rpcmetadata.ErrorClassification{
		Kind:   types.Pointerize(rpcmetadata.ErrorKind_PERMANENT),
		Blame:  types.Pointerize(rpcmetadata.ErrorBlame_CLIENT),
		Safety: types.Pointerize(rpcmetadata.ErrorSafety_SAFE),
	}
	declaredException := &rpcmetadata.PayloadExceptionMetadataBase{
		Metadata: &rpcmetadata.PayloadExceptionMetadata{
			DeclaredException: &rpcmetadata.PayloadDeclaredExceptionMetadata{
				ErrorClassification: class,
			},
		},
	}
	err := newRocketException(declaredException)
	require.NotNil(t, err)
	require.Contains(t, err.Error(), "DeclaredException")
	appUnknownException := &rpcmetadata.PayloadExceptionMetadataBase{
		Metadata: &rpcmetadata.PayloadExceptionMetadata{
			AppUnknownException: &rpcmetadata.PayloadAppUnknownExceptionMetdata{
				ErrorClassification: class,
			},
		},
	}
	err = newRocketException(appUnknownException)
	require.NotNil(t, err)
	require.Contains(t, err.Error(), "AppUnknownException")
	appUnknownException2 := &rpcmetadata.PayloadExceptionMetadataBase{
		Metadata: &rpcmetadata.PayloadExceptionMetadata{
			AppUnknownException: &rpcmetadata.PayloadAppUnknownExceptionMetdata{},
		},
	}
	err = newRocketException(appUnknownException2)
	require.NotNil(t, err)
	require.Contains(t, err.Error(), "AppUnknownException")
}
