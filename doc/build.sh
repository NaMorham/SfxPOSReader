#!/bin/bash

components=$(ls -1 ./md/*.md | sed s/\.md//g)

echo "Building man pages for SfxPOSReader..."

mkdir -p man1
mkdir -p pdf

for component in $components
do
  if [ md/${component}.md -nt man1/${component}.1 ]
  then
    echo "Building man pages for $component"
    curl -F page=@md/${component}.md http://mantastic.herokuapp.com > man1/${component}.1
    man -t -l man1/${component}.1 | ps2pdf - > pdf/${component}.pdf
  fi
done

echo "Building man pages complete"
