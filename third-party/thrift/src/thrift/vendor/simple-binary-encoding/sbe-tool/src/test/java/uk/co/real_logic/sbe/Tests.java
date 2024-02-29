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
package uk.co.real_logic.sbe;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;

public class Tests
{
    public static InputStream getLocalResource(final String name) throws IOException
    {
        final String pathToResources = System.getProperty("test.resources.dir", "");
        final URL url = Tests.class.getClassLoader().getResource(pathToResources + name);
        if (url == null)
        {
            throw new FileNotFoundException(pathToResources + name);
        }

        return url.openStream();
    }
}
