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

package com.facebook.mojo;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.StringTokenizer;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.ExecuteException;
import org.apache.commons.exec.PumpStreamHandler;
import org.apache.commons.io.FileUtils;
import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.plugin.MojoFailureException;
import org.apache.maven.plugins.annotations.LifecyclePhase;
import org.apache.maven.plugins.annotations.Mojo;
import org.apache.maven.plugins.annotations.Parameter;
import org.apache.maven.project.MavenProject;

/**
 * Compiles Thrift IDL files.
 *
 * @author ryandm, jsailor, sazonovk
 */
@Mojo(name = "thrift-generate-sources", defaultPhase = LifecyclePhase.GENERATE_SOURCES)
public class FBThriftMojo extends AbstractMojo {

  /** Base directory for includes resolution. Usually it's the root of the fbcode tree. */
  @Parameter(required = true)
  private File baseDirectory;

  /** The list of thrift source files to compile, relative to {@link #baseDirectory}. */
  @Parameter(required = true)
  private String[] sources;

  /** The generated-sources directory where output files will be put. */
  @Parameter(
      required = true,
      defaultValue = "${project.build.directory}/generated-sources/gen-java")
  private File targetDirectory;

  /** The path to and options for the thrift compiler. */
  @Parameter private Compiler compiler;

  @Parameter(defaultValue = "java-deprecated")
  private String generator;

  @Parameter(defaultValue = "${project}")
  private MavenProject project;

  /** Ignored; present for compatibility. */
  @Parameter(defaultValue = "false")
  @SuppressWarnings("unused")
  private boolean debug;

  @Override
  public void execute() throws MojoExecutionException, MojoFailureException {
    if (compiler == null) {
      compiler = new Compiler();
    }
    compiler.init();

    try {
      String currentChecksum = computeCurrentChecksum(compiler.getInfoForChecksum());
      boolean needToCompareChecksums = !compiler.hasRecursiveExtraOption();

      if (needToCompareChecksums) {
        getLog().info("Checking if " + sources.length + " thrift source files changed");
        String previousChecksum = readChecksumsFileOrNull();
        if (currentChecksum.equals(previousChecksum)) {
          getLog().info("Nothing changed, so no need to re-generate anything");
        } else {
          getLog()
              .info(
                  "Something changed; generating Java code from "
                      + sources.length
                      + " thrift files");
          compileAllAndWriteChecksum(targetDirectory, currentChecksum);
        }
      } else {
        getLog().info("Compiler is run with \"-r\" option, so all files will be re-generated");
        compileAllAndWriteChecksum(targetDirectory, currentChecksum);
      }

      project.addCompileSourceRoot(targetDirectory.getPath());
      getLog().info("add-source " + targetDirectory.getPath());
    } catch (final IOException e) {
      throw new MojoExecutionException(e.getMessage(), e);
    }
  }

  protected void compileAllAndWriteChecksum(File genJavaDirectory, String currentChecksum)
      throws IOException, MojoExecutionException {
    compileAll(genJavaDirectory);
    writeChecksumsFile(currentChecksum);
    getLog().debug("Thrift compilation complete");
  }

  private void compileAll(File genJavaDirectory) throws MojoExecutionException, IOException {
    FileUtils.deleteDirectory(genJavaDirectory);
    genJavaDirectory.mkdirs();

    for (String fbcodeSourceFile : sources) {
      compileOneFile(fbcodeSourceFile);
    }
  }

  private void compileOneFile(String fbcodeSourceFile) throws MojoExecutionException, IOException {
    final File file = new File(baseDirectory, fbcodeSourceFile);
    if (!file.exists()) {
      throw new MojoExecutionException("!file.exists(): " + file);
    }
    if (!file.isFile()) {
      throw new MojoExecutionException("!file.isFile(): " + file);
    }

    getLog().debug("Compiling " + file);
    CommandLine cl = compiler.getCompilerCommandLine();
    cl.addArgument("-I").addArgument(baseDirectory.getPath(), false);
    cl.addArgument("-gen").addArgument(generator);
    cl.addArgument("-out").addArgument(targetDirectory.getPath(), false);
    cl.addArgument(file.getPath(), false);
    DefaultExecutor executor = new DefaultExecutor();
    ByteArrayOutputStream stdout = new ByteArrayOutputStream();
    ByteArrayOutputStream stderr = new ByteArrayOutputStream();
    executor.setStreamHandler(new PumpStreamHandler(stdout, stderr));
    try {
      executor.execute(cl);
    } catch (ExecuteException e) {
      getLog()
          .error("Thrift compilation of " + file + " failed with exit code " + e.getExitValue());
      getLog().error("Command was: " + cl.toString());
      getLog().error(formatStream("Standard out", stdout));
      getLog().error(formatStream("Standard err", stderr));
      throw new MojoExecutionException("thrift execution failed", e);
    }
    if (stderr.size() > 0) {
      getLog().warn(formatStream("During compilation of " + file, stderr));
    }
  }

  private static CharSequence formatStream(CharSequence title, ByteArrayOutputStream stream) {
    if (stream.size() == 0) {
      return title + " empty";
    }
    StringBuilder out = new StringBuilder(title.length() + stream.size() + 10);
    out.append(title + ":");
    StringTokenizer tok = new StringTokenizer(stream.toString(), "\n");
    while (tok.hasMoreTokens()) {
      out.append("\n ");
      out.append(tok.nextToken());
    }
    return out;
  }

  private String computeCurrentChecksum(final String thriftCommand) throws IOException {
    StringBuilder sb = new StringBuilder().append(thriftCommand).append('\n');
    Arrays.sort(sources); // ugh
    for (String fbcodeSourceFile : sources) {
      byte[] bytes = FileUtils.readFileToByteArray(new File(baseDirectory, fbcodeSourceFile));
      sb.append(fbcodeSourceFile).append(':').append(DigestUtils.md5Hex(bytes)).append('\n');
    }
    return sb.toString();
  }

  private String readChecksumsFileOrNull() throws IOException {
    File file = getChecksumsFile();
    if (file.exists()) {
      return FileUtils.readFileToString(file, StandardCharsets.UTF_8);
    } else {
      return null;
    }
  }

  private void writeChecksumsFile(String newContents) throws IOException {
    FileUtils.write(getChecksumsFile(), newContents, StandardCharsets.UTF_8);
  }

  private File getChecksumsFile() {
    return new File(targetDirectory, "gen-java-checksums");
  }
}
