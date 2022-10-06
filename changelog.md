# Changelog

## Version 1.1

Release 06.10.2022

The cumbersome reading and writing of binary images was replaced by GDAL functionality. 
Input format is now flexible. 
Number of rows, columns, bands are no input arguments anymore.
This also fixes the missing geoinformation in the previous ENVI header.

New output format is LZW-compressed GeoTiff.

GDAL is now a requirement.

