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

package com.facebook.thrift;

import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.javaswift.test.MySimpleBean;
import org.junit.Test;

public class BeansTest {
  @Test
  public void testJavaBeans() throws Exception {
    MySimpleBean bean = new MySimpleBean();
    bean.setId(1L);
    bean.setName("Toto");

    assertThat(bean.getId(), equalTo(1L));
    assertThat(bean.getName(), equalTo("Toto"));

    bean.setId(2);
    bean.setName("Titi");
    assertThat(bean.getId(), equalTo(2L));
    assertThat(bean.getName(), equalTo("Titi"));
  }
}
