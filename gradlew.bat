@rem Gradle startup script for Windows
@if "%DEBUG%"=="" @echo off
@rem Set local scope
setlocal
set APP_HOME=%~dp0
set CLASSPATH=%APP_HOME%\gradle\wrapper\gradle-wrapper.jar
java %DEFAULT_JVM_OPTS% -classpath "%CLASSPATH%" org.gradle.wrapper.GradleWrapperMain %*
