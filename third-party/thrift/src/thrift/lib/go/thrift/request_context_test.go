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
	"context"
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"golang.org/x/sync/errgroup"
)

func TestParallelReadHeader(t *testing.T) {
	ctxHeader := &contextHeaders{}
	eg, _ := errgroup.WithContext(context.Background())

	// parallel writes
	for i := 0; i < 10; i++ {
		index := i
		value := fmt.Sprintf("VALUE-%d", index)
		eg.Go(func() error {
			for j := 0; j < 100; j++ {
				key := fmt.Sprintf("KEY-%d", j)
				ctxHeader.SetReadHeader(key, value)
				actual, ok := ctxHeader.GetReadHeader(key)
				assert.True(t, ok)
				assert.NotEmptyf(t, actual, "EMPTY FOR KEY %s", key)
			}
			return nil
		})
	}

	err := eg.Wait()
	require.NoError(t, err)
	assert.Len(t, ctxHeader.ReadHeaders(), 100)
	// Test single threaded correction
	for i := 0; i < 100; i++ {
		key := fmt.Sprintf("KEY-%d", i)
		_, ok := ctxHeader.GetReadHeader(key)
		assert.True(t, ok)
	}

	// Test increasing readHeader map
	ctxHeader.SetReadHeaders(map[string]string{})
	for i := 0; i < 10; i++ {
		index := i
		eg.Go(func() error {
			for j := 1; j <= 10; j++ {
				key := fmt.Sprintf("KEY-%d", (index*10)+j)
				value := fmt.Sprintf("VALUE-%d", (index*10)+j)
				ctxHeader.SetReadHeader(key, value)
				actual, ok := ctxHeader.GetReadHeader(key)
				assert.True(t, ok)
				assert.Equal(t, value, actual)
			}
			return nil
		})
	}

	err = eg.Wait()
	require.NoError(t, err)
	expectedReadHeaderMap := map[string]string{}
	for i := 1; i <= 100; i++ {
		key := fmt.Sprintf("KEY-%d", i)
		value := fmt.Sprintf("VALUE-%d", i)
		expectedReadHeaderMap[key] = value
	}
	actualReadHeaderMap := ctxHeader.ReadHeaders()
	assert.Len(t, actualReadHeaderMap, len(expectedReadHeaderMap))
	for expectedKey, expectedValue := range expectedReadHeaderMap {
		assert.Equal(t, expectedValue, actualReadHeaderMap[expectedKey])
	}

	// Test nonexistent key
	value, ok := ctxHeader.GetReadHeader("FAKE-KEY")
	assert.False(t, ok)
	assert.Empty(t, value)
}

func TestParallelWriteHeader(t *testing.T) {
	ctxHeader := &contextHeaders{}
	eg, _ := errgroup.WithContext(context.Background())

	// parallel writes
	for i := 0; i < 10; i++ {
		index := i
		value := fmt.Sprintf("VALUE-%d", index)
		eg.Go(func() error {
			for j := 0; j < 100; j++ {
				key := fmt.Sprintf("KEY-%d", j)
				ctxHeader.SetWriteHeader(key, value)
				actual, ok := ctxHeader.GetWriteHeader(key)
				assert.True(t, ok)
				assert.NotEmptyf(t, actual, "EMPTY FOR KEY %s", key)
			}
			return nil
		})
	}

	err := eg.Wait()
	require.NoError(t, err)
	assert.Len(t, ctxHeader.WriteHeaders(), 100)
	// Test single threaded correction
	for i := 0; i < 100; i++ {
		key := fmt.Sprintf("KEY-%d", i)
		_, ok := ctxHeader.GetWriteHeader(key)
		assert.True(t, ok)
	}

	// Test increasing WriteHeader map
	ctxHeader.SetWriteHeaders(map[string]string{})
	for i := 0; i < 10; i++ {
		index := i
		eg.Go(func() error {
			for j := 1; j <= 10; j++ {
				key := fmt.Sprintf("KEY-%d", (index*10)+j)
				value := fmt.Sprintf("VALUE-%d", (index*10)+j)
				ctxHeader.SetWriteHeader(key, value)
				actual, ok := ctxHeader.GetWriteHeader(key)
				assert.True(t, ok)
				assert.Equal(t, value, actual)
			}
			return nil
		})
	}

	err = eg.Wait()
	require.NoError(t, err)
	expectedWriteHeaderMap := map[string]string{}
	for i := 1; i <= 100; i++ {
		key := fmt.Sprintf("KEY-%d", i)
		value := fmt.Sprintf("VALUE-%d", i)
		expectedWriteHeaderMap[key] = value
	}
	actualWriteHeaderMap := ctxHeader.WriteHeaders()
	assert.Len(t, actualWriteHeaderMap, len(expectedWriteHeaderMap))
	for expectedKey, expectedValue := range expectedWriteHeaderMap {
		assert.Equal(t, expectedValue, actualWriteHeaderMap[expectedKey])
	}

	// Test nonexistent key
	value, ok := ctxHeader.GetReadHeader("FAKE-KEY")
	assert.False(t, ok)
	assert.Empty(t, value)
}
