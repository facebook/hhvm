package com.facebook.mojo;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.PumpStreamHandler;
import org.apache.maven.plugin.MojoExecutionException;
import org.codehaus.plexus.util.StringUtils;

public class Compiler {
  /**
   * The absolute path to the thrift compiler.
   *
   * @parameter
   */
  @SuppressWarnings("unused")
  private String absolutePath;

  /**
   * A relative path to the thrift compiler, specified in Maven's standard format.
   *
   * @parameter
   */
  @SuppressWarnings("unused")
  private File relativePath;

  /**
   * Extra options to pass to the thrift compiler.
   *
   * @parameter
   */
  @SuppressWarnings("unused")
  private String[] extraOptions;

  /**
   * Try to find the thrift compiler by checking <code>$PATH</code>.
   *
   * @param commandName the basename of a command to search for, e.g. "thrift"
   * @return a {@link File} pointing to the found command, or null if it wasn't
   */
  private static File tryFromPath(String commandName) {
    final DefaultExecutor executor = new DefaultExecutor();
    final ByteArrayOutputStream stdout = new ByteArrayOutputStream();
    final ByteArrayOutputStream stderr = new ByteArrayOutputStream();
    executor.setStreamHandler(new PumpStreamHandler(stdout, stderr));

    CommandLine cli = new CommandLine("sh").addArgument("-c");
    cli.addArgument(
        "command -v " + commandName, // <-- semantics defined by POSIX
        false // commons-exec is a piece of shit: https://issues.apache.org/jira/browse/EXEC-54
        );
    try {
      executor.execute(cli);
    } catch (IOException e) {
      return null;
    }

    String compilerStr = stdout.toString();
    compilerStr = StringUtils.chopNewline(compilerStr);
    if (StringUtils.isEmpty(compilerStr)) return null;

    File compiler = new File(compilerStr);
    if (compiler.exists()) return compiler;
    else return null;
  }

  private File computeCompiler() throws MojoExecutionException {
    File resolvedPath;

    if (StringUtils.isNotEmpty(absolutePath) && relativePath != null) {
      throw new MojoExecutionException("Both absolutePath and relativePath are specified");
    }

    if (StringUtils.isNotEmpty(absolutePath)) {
      resolvedPath = new File(absolutePath);
    } else if (relativePath != null) {
      resolvedPath = relativePath;
    } else {
      File fromPath = tryFromPath("thrift");
      if (fromPath != null) {
        resolvedPath = fromPath;
      } else {
        fromPath = tryFromPath("thrift1");
        if (fromPath != null) {
          resolvedPath = fromPath;
        } else {
          throw new MojoExecutionException("Couldn't find thrift compiler");
        }
      }
    }

    if (!resolvedPath.exists()) {
      throw new MojoExecutionException("Thrift compiler '" + resolvedPath + "' does not exist");
    }

    return resolvedPath;
  }

  /** Memoized version of {@link #computeCompiler()} */
  private File computedCompiler;

  /**
   * Prepares this object to run the thrift compiler.
   *
   * @throws MojoExecutionException
   */
  public void init() throws MojoExecutionException {
    computedCompiler = computeCompiler();
  }

  public CommandLine getCompilerCommandLine() {
    CommandLine cli = new CommandLine(computedCompiler);
    if (extraOptions != null) cli.addArguments(extraOptions);
    return cli;
  }

  public String getInfoForChecksum() {
    StringBuilder sb = new StringBuilder(computedCompiler.getAbsolutePath());
    if (extraOptions != null) {
      for (String extra : extraOptions) {
        sb.append(" ").append(extra);
      }
    }
    return sb.toString();
  }

  public boolean hasExtraOption(String compilerOption) {
    if (extraOptions != null) return Arrays.asList(extraOptions).contains(compilerOption);
    else return false;
  }

  public boolean hasRecursiveExtraOption() {
    return hasExtraOption("-r") || hasExtraOption("--recursive");
  }
}
