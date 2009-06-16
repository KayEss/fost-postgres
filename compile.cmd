@echo off
..\bjam toolset=msvc %*

IF NOT ERRORLEVEL 1 (
    call ..\boost-version.cmd
    for /r %BUILD_DIRECTORY% %%f in (*.pdb) do xcopy /D /Y %%f ..\dist\bin
)

IF NOT ERRORLEVEL 1 (
    ..\dist\bin\fost-schema-test-dyndriver -b false pqxx fost-postgres.dll -d "user=Test password=tester host=localhost dbname=postgres"
)

IF NOT ERRORLEVEL 1 (
    ..\dist\bin\ftest -b false -i Configuration/postgres-windows.ini ..\dist\bin\fost-postgres-test-smoke.dll
)
