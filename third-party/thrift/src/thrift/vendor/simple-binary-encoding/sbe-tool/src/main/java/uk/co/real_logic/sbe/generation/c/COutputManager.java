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
package uk.co.real_logic.sbe.generation.c;

import org.agrona.generation.OutputManager;
import org.agrona.Verify;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;

import static java.io.File.separatorChar;

/**
 * {@link OutputManager} for managing the creation of C11 source files as the target of code generation.
 * The character encoding for the {@link java.io.Writer} is UTF-8.
 */
public class COutputManager implements OutputManager
{
    private final File outputDir;

    /**
     * Create a new {@link OutputManager} for generating C11 source files into a given package.
     *
     * @param baseDirName   for the generated source code.
     * @param namespaceName for the generated source code relative to the baseDirName.
     */
    public COutputManager(final String baseDirName, final String namespaceName)
    {
        Verify.notNull(baseDirName, "baseDirName");
        Verify.notNull(namespaceName, "applicableNamespace");

        final String dirName = baseDirName.endsWith("" + separatorChar) ? baseDirName : baseDirName + separatorChar;
        final String packageDirName = dirName + namespaceName.replace('.', '_');

        outputDir = new File(packageDirName);
        if (!outputDir.exists() && !outputDir.mkdirs())
        {
            throw new IllegalStateException("Unable to create directory: " + packageDirName);
        }
    }

    /**
     * Create a new output which will be a C11 source file in the given package.
     * <p>
     * The {@link java.io.Writer} should be closed once the caller has finished with it. The Writer is
     * buffered for efficient IO operations.
     *
     * @param name the name of the C struct.
     * @return a {@link java.io.Writer} to which the source code should be written.
     * @throws IOException if an issue occurs when creating the file.
     */
    public Writer createOutput(final String name) throws IOException
    {
        final File targetFile = new File(outputDir, name + ".h");
        return Files.newBufferedWriter(targetFile.toPath(), StandardCharsets.UTF_8);
    }
}
