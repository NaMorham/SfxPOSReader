SfxPOSReader(1) -- Extraxt XYZ data from StarFix MBE pos assets into S3.
===========================================

## ABOUT

SfxPOSReader is a command line utility for extracting XYZ data from StarFix MBE pos data files.

## SYNOPSIS

SfxPOSReader \[**options**\] <**input\_path**> <**output\_path**>

## OPTIONS

The following options may be provided:

  **<-xyzfile>**
        The name of the output XYZ file

  **[TBA]**
        The number of bytes to transfer

  **[-numthreads %d]**
        The number of threads to use for multipart upload [default = 1].

  **[-loglevel %d]**
        Logging verbosity 0:None 1:Error 2:Warning 3:Info 4:Debug [2]

  **[-help]**
        Print the help message and usage information

## USAGE

TBA
  `$ SfxPOSReader -xzyfile /tmp/wooot.xyz in1.pos in2.pos`

## AUTHOR

Written by Andrew Hickey

## BUGS

Please report bugs via JIRA - https://roames.jira.com

## COPYRIGHT

Copyright 2015 - Fugro Roames

