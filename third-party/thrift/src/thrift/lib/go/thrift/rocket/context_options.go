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
)

type rocketOptionsKeyType int

const (
	frameworkMetadataKey rocketOptionsKeyType = 1
)

// WithFrameworkMetadata sets the framework metadata for the RPC.
func WithFrameworkMetadata(ctx context.Context, metadata []byte) context.Context {
	return context.WithValue(ctx, frameworkMetadataKey, metadata)
}

func getFrameworkMetadata(ctx context.Context) []byte {
	metadata, ok := ctx.Value(frameworkMetadataKey).([]byte)
	if ok {
		return metadata
	}
	return nil
}
