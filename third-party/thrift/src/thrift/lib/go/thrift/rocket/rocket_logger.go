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
	"github.com/rsocket/rsocket-go/logger"
)

type rsocketLogger func(format string, args ...any)

// Debugf print to the debug level logs.
func (log rsocketLogger) Debugf(format string, args ...any) {
	log(format, args...)
}

// Infof print to the info level logs.
func (log rsocketLogger) Infof(format string, args ...any) {
	log(format, args...)
}

// Warnf print to the info level logs.
func (log rsocketLogger) Warnf(format string, args ...any) {
	log(format, args...)
}

// Errorf print to the info level logs.
func (log rsocketLogger) Errorf(format string, args ...any) {
	log(format, args...)
}

// SetRsocketLogger sets the logger for rsocket-go.
func SetRsocketLogger(log func(format string, args ...any)) {
	logger.SetLogger(rsocketLogger(log))
}

func init() {
	// Set the default logger to error level.
	logger.SetLevel(logger.LevelError)
}
