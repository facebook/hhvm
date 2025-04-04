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
	"os"
	"os/exec"
	"strconv"
	"strings"
	"testing"

	"github.com/stretchr/testify/require"
)

func getNumFileDesciptors(t *testing.T) int {
	cmd := exec.Command("lsof", "-p", strconv.Itoa(os.Getpid()))
	output, err := cmd.CombinedOutput()
	require.NoError(t, err)

	lines := strings.Split(string(output), "\n")
	require.Greater(t, len(lines), 1)

	return len(lines) - 1 // -1 to account for header line
}
