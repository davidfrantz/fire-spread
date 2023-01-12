# Changelog

## Version 1.6

Release 12.01.2023

Worked on output. 
Geopackage is additionally written.
A basic csv file is written (same information as geopackage).
An extended csv file is written, which includes the full fire-spread information - can be joined with basic csv and/or geopackage.


## Version 1.2

Release 17.11.2022

Number of CPUs can be specified.


## Version 1.1

Release 06.10.2022

The cumbersome reading and writing of binary images was replaced by GDAL functionality. 
Input format is now flexible. 
Number of rows, columns, bands are no input arguments anymore.
This also fixes the missing geoinformation in the previous ENVI header.

New output format is LZW-compressed GeoTiff.

GDAL is now a requirement.

