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
package uk.co.real_logic.sbe.codec.java;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Selects sorted methods of a class that are annotated with {@link GroupOrder}.
 */
public class MethodSelector
{
    private final Set<String> ignoredMethods;
    private final Map<Class<?>, Set<String>> sortedMethods = new HashMap<>();

    /**
     * The method names for method belonging to Object and Iterator.
     *
     * @return the method names for method belonging to Object and Iterator.
     */
    public static Set<String> objectAndIteratorMethods()
    {
        return new HashSet<>(
            Arrays.asList("hashCode", "clone", "toString", "getClass", "next", "hasNext", "remove", "iterator"));
    }

    /**
     * Create a method selector with a set of methods to be ignored.
     *
     * @param ignoredMethods to be skipped over.
     */
    public MethodSelector(final Set<String> ignoredMethods)
    {
        this.ignoredMethods = ignoredMethods;
    }

    /**
     * Select a list of methods from a given class.
     *
     * @param clazz to select the method for.
     * @return the list of methods
     */
    public List<Method> select(final Class<?> clazz)
    {
        final Method[] methods = clazz.getMethods();
        final Set<String> sortedMethodNames = getSortedMethods(clazz, methods);
        final Map<String, Method> sortedMethods = new HashMap<>();
        final List<Method> unsortedMethods = new ArrayList<>();

        for (final Method method : methods)
        {
            selectMethod(sortedMethodNames, sortedMethods, unsortedMethods, method);
        }

        for (final String name : sortedMethodNames)
        {
            unsortedMethods.add(sortedMethods.get(name));
        }

        return unsortedMethods;
    }

    private Set<String> getSortedMethods(final Class<?> clazz, final Method[] methods)
    {
        final Set<String> sortedMethodNames = sortedMethods.get(clazz);
        if (null == sortedMethodNames)
        {
            final GroupOrder order = clazz.getAnnotation(GroupOrder.class);
            if (null == order)
            {
                sortedMethods.put(clazz, Collections.emptySet());

                return Collections.emptySet();
            }
            else
            {
                final Set<String> result = new LinkedHashSet<>();
                for (final Class<?> groupClazz : order.value())
                {
                    for (final Method method : methods)
                    {
                        if (method.getReturnType() == groupClazz && method.getParameterTypes().length == 0)
                        {
                            result.add(method.getName());
                        }
                    }
                }
                sortedMethods.put(clazz, result);

                return result;
            }
        }

        return sortedMethodNames;
    }

    private void selectMethod(
        final Set<String> sortedMethodNames,
        final Map<String, Method> sortedMethods,
        final List<Method> unsortedMethods,
        final Method method)
    {
        final int mods = method.getModifiers();
        if (!Modifier.isPublic(mods) ||
            Modifier.isStatic(mods) ||
            method.getParameterTypes().length != 0 ||
            method.getReturnType().equals(Void.TYPE) ||
            ignoredMethods.contains(method.getName()))
        {
            return;
        }

        if (null == sortedMethodNames)
        {
            unsortedMethods.add(method);
        }
        else
        {
            if (sortedMethodNames.contains(method.getName()))
            {
                sortedMethods.put(method.getName(), method);
            }
            else
            {
                unsortedMethods.add(method);
            }
        }
    }
}
