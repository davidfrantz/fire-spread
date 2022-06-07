# fire-spread

## Background
For details on methods, please, refer to our paper:
*Frantz David, Stellmes Marion, Röder Achim, Hill Joachim (2016) Fire spread from MODIS burned area data: obtaining fire dynamics information for every single fire. International Journal of Wildland Fire 25, 1228-1237. https://www.publish.csiro.au/wf/WF16003*

## Usage

*fire-spread input-stack input-dates output-dir basename nx ny nb
  init-searchdist track-searchdist temp-dist density-dist min-size smooth-dist verbose*
  
|Argument| Short Description|
|:---|:---|
|input-stack|input data (see below)|
|input-dates|list of input dates (see below)|
|output-dir|output directory|
|basename|basename, which is used for naming output|
|nx|number of columns|
|ny|number of lines|
|nb|number of bands (= months)|
|init-searchdist|see paper|
|track-searchdist|see paper|
|temp-dist|see paper|
|density-dist|see paper|
|min-size|see paper|
|smooth-dist|see paper|
|verbose|print detailed information (v) or be quiet (q)|

## Input data
### input-stack
* monthly stack of burned area data (DOY)
* mosaic of complete area
* reprojected to some projection which minimizes distortion of both area and angles (e.g. equidistant, see https://doi.org/10.1016/j.cageo.2014.07.005)
* 16bit unsigned ENVI Standard

**Hints:**
* We developed the code for the MODIS burned area product MCD45A1 collection 5.1 but used it also successfully for MCD64 collection 6 (https://modis-fire.umd.edu/guides.html) and the ESA CCI data versio) 5.1 (https://geogra.uah.es/fire_cci/firecci51.php) 
* The different fire seasons (one per year) will be automatically identified, and parallel processing (one CPU per season) will be used for speeding things up.

### input-dates
* text file, which contains the dates of the input-stack as YYYYMM
* each date in one line
* last line should be empty
* use Unix EOL

## 


