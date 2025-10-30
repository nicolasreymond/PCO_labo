#!/bin/bash

SCRIPT=${BASH_SOURCE[0]}
SCRIPTPATH=$(realpath $(dirname $SCRIPT))
PROJECT_DIR=common
ARCHIVE=rendu.tar.gz
REPORT_FILE=rapport.pdf

if [ ! -d "$PROJECT_DIR" ]
then
    echo "Could not find $PROJECT_DIR directory in $(pwd)" >&2
    exit 1
fi

if [ ! -f "$REPORT_FILE" ]
then
    echo "Could not find $REPORT_FILE directory in $SCRIPTPATH" >&2
    exit 1
fi


echo "The following files are archived in $ARCHIVE : "
tar --exclude='rendu.tar.gz' --exclude='*.o' --exclude='*.user' -czvf $ARCHIVE ${PROJECT_DIR} $REPORT_FILE

