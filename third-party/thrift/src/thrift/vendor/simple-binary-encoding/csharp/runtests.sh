#!/bin/bash
DIR="$(dirname "${BASH_SOURCE[0]}")"
cd "${DIR}" || exit
(cd ../; ./gradlew runExampleUsingGeneratedStub -Dsbe.encoding.filename=csharp/sbe-tests/ExampleUsingGeneratedStub.sbe)
(cd sbe-tests && dotnet test)
