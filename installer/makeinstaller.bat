@ECHO OFF
REM This script creates the installer for Proximodo on Windows. 
REM It has only been tested on Windows XP but should work on most Windows flavours.
REM You have to set the NSIS_HOME environment variable before running the script.

"%NSIS_HOME%\makensis.exe" proximodo.nsi