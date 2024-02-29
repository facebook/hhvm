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
package uk.co.real_logic.sbe.generation;

import uk.co.real_logic.sbe.generation.c.CGenerator;
import uk.co.real_logic.sbe.generation.c.COutputManager;
import uk.co.real_logic.sbe.generation.common.PrecedenceChecks;
import uk.co.real_logic.sbe.generation.cpp.CppGenerator;
import uk.co.real_logic.sbe.generation.cpp.NamespaceOutputManager;
import uk.co.real_logic.sbe.generation.golang.GolangGenerator;
import uk.co.real_logic.sbe.generation.golang.GolangOutputManager;
import uk.co.real_logic.sbe.generation.java.JavaGenerator;
import uk.co.real_logic.sbe.generation.java.JavaOutputManager;
import uk.co.real_logic.sbe.generation.rust.RustGenerator;
import uk.co.real_logic.sbe.generation.rust.RustOutputManager;
import uk.co.real_logic.sbe.ir.Ir;

import static uk.co.real_logic.sbe.SbeTool.*;

/**
 * Loader for {@link CodeGenerator}s which target a language. This provides convenient short names rather than the
 * fully qualified class name of the generator.
 */
public enum TargetCodeGeneratorLoader implements TargetCodeGenerator
{
    /**
     * Generates codecs for the Java 8 programming language.
     */
    JAVA()
    {
        /**
         * {@inheritDoc}
         */
        public CodeGenerator newInstance(final Ir ir, final String outputDir)
        {
            return new JavaGenerator(
                ir,
                System.getProperty(JAVA_ENCODING_BUFFER_TYPE, JAVA_DEFAULT_ENCODING_BUFFER_TYPE),
                System.getProperty(JAVA_DECODING_BUFFER_TYPE, JAVA_DEFAULT_DECODING_BUFFER_TYPE),
                "true".equals(System.getProperty(JAVA_GROUP_ORDER_ANNOTATION)),
                "true".equals(System.getProperty(JAVA_GENERATE_INTERFACES)),
                "true".equals(System.getProperty(DECODE_UNKNOWN_ENUM_VALUES)),
                "true".equals(System.getProperty(TYPES_PACKAGE_OVERRIDE)),
                precedenceChecks(),
                new JavaOutputManager(outputDir, ir.applicableNamespace()));
        }
    },

    /**
     * Generates codecs for the C11 programming language.
     */
    C()
    {
        /**
         * {@inheritDoc}
         */
        public CodeGenerator newInstance(final Ir ir, final String outputDir)
        {
            return new CGenerator(ir, new COutputManager(outputDir, ir.applicableNamespace()));
        }
    },

    /**
     * Generates codecs for the C++11 programming language with some conditional includes for C++14 and C++17.
     */
    CPP()
    {
        /**
         * {@inheritDoc}
         */
        public CodeGenerator newInstance(final Ir ir, final String outputDir)
        {
            return new CppGenerator(
                ir,
                "true".equals(System.getProperty(DECODE_UNKNOWN_ENUM_VALUES)),
                precedenceChecks(),
                new NamespaceOutputManager(outputDir, ir.applicableNamespace()));
        }
    },

    /**
     * Generates codecs for the Go programming language.
     */
    GOLANG()
    {
        /**
         * {@inheritDoc}
         */
        public CodeGenerator newInstance(final Ir ir, final String outputDir)
        {
            return new GolangGenerator(ir, new GolangOutputManager(outputDir, ir.applicableNamespace()));
        }
    },

    /**
     * Generates codecs for the Rust programming language.
     */
    RUST()
    {
        /**
         * {@inheritDoc}
         */
        public CodeGenerator newInstance(final Ir ir, final String outputDir)
        {
            return new RustGenerator(
                ir,
                new RustOutputManager(outputDir, ir.packageName()));
        }
    };

    /**
     * Returns the precedence checks to run, configured from system properties.
     *
     * @return the precedence checks to run, configured from system properties.
     */
    public static PrecedenceChecks precedenceChecks()
    {
        final PrecedenceChecks.Context context = new PrecedenceChecks.Context();

        final String shouldGeneratePrecedenceChecks = System.getProperty(GENERATE_PRECEDENCE_CHECKS);
        if (shouldGeneratePrecedenceChecks != null)
        {
            context.shouldGeneratePrecedenceChecks(Boolean.parseBoolean(shouldGeneratePrecedenceChecks));
        }

        final String precedenceChecksFlagName = System.getProperty(PRECEDENCE_CHECKS_FLAG_NAME);
        if (precedenceChecksFlagName != null)
        {
            context.precedenceChecksFlagName(precedenceChecksFlagName);
        }

        final String precedenceChecksPropName = System.getProperty(JAVA_PRECEDENCE_CHECKS_PROPERTY_NAME);
        if (precedenceChecksPropName != null)
        {
            context.precedenceChecksPropName(precedenceChecksPropName);
        }

        return PrecedenceChecks.newInstance(context);
    }

    /**
     * Do a case-insensitive lookup of a target language for code generation.
     *
     * @param name of the target language to lookup.
     * @return the {@link TargetCodeGenerator} for the given language name.
     */
    public static TargetCodeGenerator get(final String name)
    {
        for (final TargetCodeGeneratorLoader target : values())
        {
            if (target.name().equalsIgnoreCase(name))
            {
                return target;
            }
        }

        try
        {
            return (TargetCodeGenerator)Class.forName(name).getConstructor().newInstance();
        }
        catch (final Exception ex)
        {
            throw new IllegalArgumentException("No code generator for name: " + name, ex);
        }
    }
}
