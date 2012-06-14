#!/bin/sh
xvfb-run ctest
PROC_RET=$?
make reporthtml
REVISION=`git rev-parse --short HEAD`
mv reporthtml $REVISION
zip $REVISION.zip $REVISION
curl -F zip_file=@$REVISION.zip  http://prereleases.musescore.org/test/index.php
echo "Test results: http://prereleasese.musescore.org/test/$REVISION/"

exit $PROC_RET