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
package uk.co.real_logic.sbe.generation.java;

import org.junit.jupiter.api.Test;
import org.agrona.generation.PackageOutputManager;

import java.io.File;
import java.io.Writer;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertTrue;

class PackageOutputManagerTest
{
    private final String tempDirName = System.getProperty("java.io.tmpdir");

    @Test
    void shouldCreateFileWithinPackage() throws Exception
    {
        final String packageName = "uk.co.real_logic.test";
        final String exampleClassName = "ExampleClassName";

        final PackageOutputManager pom = new PackageOutputManager(tempDirName, packageName);
        final Writer out = pom.createOutput(exampleClassName);
        out.close();

        final String baseDirName = tempDirName.endsWith("" + File.separatorChar) ?
            tempDirName : tempDirName + File.separatorChar;

        final String fullyQualifiedFilename = baseDirName + packageName.replace('.', File.separatorChar) +
            File.separatorChar + exampleClassName + ".java";

        final Path path = FileSystems.getDefault().getPath(fullyQualifiedFilename);
        final boolean exists = Files.exists(path);
        Files.delete(path);

        assertTrue(exists);
    }
}
