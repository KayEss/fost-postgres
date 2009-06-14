..\bjam toolset=msvc %*
@echo off
call ..\boost-version.cmd
IF NOT ERRORLEVEL 1 (
    for /r %BUILD_DIRECTORY% %%f in (*.pdb) do xcopy /D /Y %%f ..\dist\bin
)

IF NOT ERRORLEVEL 1 (
    ..\dist\bin\ftest -b false ..\dist\bin\fost-postgres-test-smoke.dll
)
