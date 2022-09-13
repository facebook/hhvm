package com.facebook.mojo.example;

public class FBCodeMavenThriftPluginExample {
  public static void main(String[] args) {
    System.out.println("Look at the pom.xml for an example of using fbcode-maven-thrift-plugin");

    Example1 example1 = new Example1();
    example1.setBar(3);
    example1.setBaz("hello");

    Example2 example2 = new Example2();
    example2.setBar(3);
    example2.setBaz("hello");

    System.out.println("This compiles and runs. Therefore there are no bugs!");
  }
}
