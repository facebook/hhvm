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

package com.facebook.mojo.example;

public class FBCodeMavenThriftPluginExample {
  public static void main(String[] args) {
    System.out.println("Look at the pom.xml for an example of using fbcode-maven-thrift-plugin");

    Example1 example1 = new Example1(3, "hello");
    System.out.println(example1.getBaz() + " " + example1.getBar());

    Example2 example2 = new Example2(2, "goodbye");
    System.out.println(example2.getBaz() + " " + example2.getBar());

    System.out.println("This compiles and runs. Therefore there are no bugs!");
  }
}
