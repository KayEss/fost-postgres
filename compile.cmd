@echo off
pushd %0\..

echo fost-postgres

..\bjam %*
IF ERRORLEVEL 1 (
    popd
    copy
) ELSE (
    popd
)
