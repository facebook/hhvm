/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "issue889/lotType.h"

class Issue889Test : public testing::Test
{
};

TEST_F(Issue889Test, shouldGenerateSpecalisedNullValue)
{
    issue889_lotType lot_type;

    EXPECT_TRUE(issue889_lotType_get(0, &lot_type));
    EXPECT_EQ(lot_type, issue889_lotType::issue889_lotType_NULL_VALUE);
}
