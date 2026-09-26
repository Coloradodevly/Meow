#!/usr/bin/env sh
##############################################################################
# Gradle start up script for UN*X
##############################################################################
APP_NAME="Gradle"
APP_BASE_NAME=$(basename "$0")
APP_HOME=$(cd "$(dirname "$0")" && pwd)

CLASSPATH=$APP_HOME/gradle/wrapper/gradle-wrapper.jar
DEFAULT_JVM_OPTS='"-Xmx64m" "-Xms64m"'

set -e
die () { echo; echo "ERROR: $*"; echo; exit 1; } >&2

command -v java >/dev/null 2>&1 || die "JAVA_HOME is not set and no 'java' command could be found."

exec java $DEFAULT_JVM_OPTS \
  -classpath "$CLASSPATH" \
  org.gradle.wrapper.GradleWrapperMain "$@"
