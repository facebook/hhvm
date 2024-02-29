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

import org.agrona.SystemUtil;
import org.junit.jupiter.api.Test;

import java.io.File;
import java.io.IOException;
import java.io.Writer;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertTrue;

class JavaOutputManagerTest
{
    @Test
    void shouldCreateFileWithinPackage() throws Exception
    {
        final String packageName = "uk.co.real_logic.test";
        final String exampleClassName = "ExampleClassName";

        final String tempDirName = SystemUtil.tmpDirName();
        final JavaOutputManager cut = new JavaOutputManager(tempDirName, packageName);
        final Writer out = cut.createOutput(exampleClassName);
        out.close();

        final String typePackageName = "uk.co.real_logic.common";
        final String typeClassName = "CompositeBigDecimal";
        cut.setPackageName(typePackageName);
        final Writer typeOut = cut.createOutput(typeClassName);
        typeOut.close();

        final String typePackageName2 = "uk.co.real_logic.common2";
        final String typeClassName2 = "CompositeBigInteger";
        cut.setPackageName(typePackageName2);
        final Writer typeOut2 = cut.createOutput(typeClassName2);
        typeOut2.close();

        final String exampleClassName2 = "ExampleClassName2";

        final Writer out2 = cut.createOutput(exampleClassName2);
        out2.close();

        assertFileExists(packageName, exampleClassName);
        assertFileExists(packageName, exampleClassName2);
        assertFileExists(typePackageName, typeClassName);
        assertFileExists(typePackageName2, typeClassName2);
    }

    private void assertFileExists(final String packageName, final String exampleClassName) throws IOException
    {
        final String tempDirName = SystemUtil.tmpDirName();
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
