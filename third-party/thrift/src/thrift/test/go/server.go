/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package gotest

import (
	"context"
	"thrift/test/go/if/thrifttest"
)

type testHandler struct {
	ReturnError error
}

func (t *testHandler) DoTestVoid(ctx context.Context) error {
	return t.ReturnError
}

func (t *testHandler) DoTestString(ctx context.Context, thing string) (string, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestByte(ctx context.Context, thing int8) (int8, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestI32(ctx context.Context, thing int32) (int32, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestI64(ctx context.Context, thing int64) (int64, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestDouble(ctx context.Context, thing float64) (float64, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestFloat(ctx context.Context, thing float32) (float32, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestStruct(ctx context.Context, thing *thrifttest.Xtruct) (*thrifttest.Xtruct, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestNest(ctx context.Context, thing *thrifttest.Xtruct2) (*thrifttest.Xtruct2, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestMap(ctx context.Context, thing map[int32]int32) (map[int32]int32, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestSet(ctx context.Context, thing []int32) ([]int32, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestList(ctx context.Context, thing []int32) ([]int32, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestEnum(ctx context.Context, thing thrifttest.Numberz) (thrifttest.Numberz, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestTypedef(ctx context.Context, thing thrifttest.UserId) (thrifttest.UserId, error) {
	return thing, t.ReturnError
}

func (t *testHandler) DoTestMapMap(ctx context.Context, hello int32) (map[int32]map[int32]int32, error) {
	res := map[int32]map[int32]int32{}
	for i := int32(0); i < hello; i++ {
		res[i] = map[int32]int32{i: i}
	}
	return res, t.ReturnError
}

func (t *testHandler) DoTestInsanity(ctx context.Context, argument *thrifttest.Insanity) (map[thrifttest.UserId]map[thrifttest.Numberz]*thrifttest.Insanity, error) {
	ret := map[thrifttest.UserId]map[thrifttest.Numberz]*thrifttest.Insanity{}
	ret[thrifttest.UserId(3)] = map[thrifttest.Numberz]*thrifttest.Insanity{
		thrifttest.Numberz_EIGHT: argument,
	}
	return ret, t.ReturnError
}

func (t *testHandler) DoTestMulti(
	ctx context.Context,
	arg0 int8, arg1 int32, arg2 int64, arg3 map[int16]string,
	arg4 thrifttest.Numberz, arg5 thrifttest.UserId,
) (*thrifttest.Xtruct, error) {
	xs := thrifttest.NewXtruct()
	xs.ByteThing = arg0
	xs.I32Thing = arg1
	xs.I64Thing = arg2
	return xs, t.ReturnError
}

func (t *testHandler) DoTestException(ctx context.Context, arg string) error {
	return t.ReturnError
}

func (t *testHandler) DoTestMultiException(ctx context.Context, arg0, arg1 string) (*thrifttest.Xtruct, error) {
	xs := thrifttest.NewXtruct()
	xs.StringThing = arg0
	return xs, t.ReturnError
}

func (t *testHandler) DoTestOneway(ctx context.Context, secondsToSleep int32) error {
	return t.ReturnError
}

func (t *testHandler) XDoTestPoorName(ctx context.Context) error {
	return t.ReturnError
}
