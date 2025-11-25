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
	"context"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestContextOptions(t *testing.T) {
	t.Run("empty context", func(t *testing.T) {
		frameworkMetadata := getFrameworkMetadata(context.TODO())
		assert.Nil(t, frameworkMetadata)
	})
	t.Run("non-empty context", func(t *testing.T) {
		frameworkMetadata := []byte{1, 2, 3}
		ctx := WithFrameworkMetadata(context.TODO(), frameworkMetadata)
		frameworkMetadataPrime := getFrameworkMetadata(ctx)
		assert.Equal(t, frameworkMetadata, frameworkMetadataPrime)
	})
}
