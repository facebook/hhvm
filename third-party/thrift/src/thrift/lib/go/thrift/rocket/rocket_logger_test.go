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
	"bytes"
	"fmt"
	"testing"

	"github.com/rsocket/rsocket-go/logger"
	"github.com/stretchr/testify/require"
)

func TestRocketLogger(t *testing.T) {
	var buffer bytes.Buffer

	logFn := func(format string, args ...any) {
		fmt.Fprintf(&buffer, format, args...)
	}
	SetRsocketLogger(logFn)

	logger := rsocketLogger(logFn)
	logger.Debugf("%s", "debug\n")
	logger.Infof("%s", "info\n")
	logger.Warnf("%s", "warn\n")
	logger.Errorf("%s", "error\n")

	require.Equal(t, "debug\ninfo\nwarn\nerror\n", buffer.String())
}

func TestRsocketLogLevel(t *testing.T) {
	// Ensure that the default log level is Error.
	// This prevents noisy/verbose logs.
	level := logger.GetLevel()
	require.Equal(t, logger.LevelError, level)
}
