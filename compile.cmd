@echo off
echo fost-postgres
..\bjam toolset=msvc %*

IF NOT ERRORLEVEL 1 (
    ..\dist\bin\fost-schema-test-dyndriver -b false pqxx fost-postgres.dll -d "user=Test password=tester host=localhost dbname=postgres"
)
