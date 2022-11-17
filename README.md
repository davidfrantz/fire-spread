# fire-spread

**Version 1.1**

![fire-spread Logo](/images/fire-spread.png)


## Background

Fire spread information on a large scale is still a missing key layer for a complete description of fire regimes.
We developed a novel multilevel object-based methodology that extracts valuable information about fire dynamics from
Moderate Resolution Imaging Spectroradiometer (MODIS) burned area data. Besides the large area capabilities, this
approach also derives very detailed information for every single fire regarding timing and location of its ignition, as well as
detailed directional multitemporal spread information. The approach is a top–down approach and a multilevel
segmentation strategy is used to gradually refine the individual object membership. The multitemporal segmentation
alternates between recursive seed point identification and queue-based fire tracking. The algorithm relies on only a few
input parameters that control the segmentation with spatial and temporal distance thresholds. 

For details on methods, please, refer to our paper:

David Frantz, Marion Stellmes, Achim Röder, Joachim Hill (2016): **Fire spread from MODIS burned area data: obtaining fire dynamics information for every single fire**. *International Journal of Wildland Fire* 25, 1228-1237. https://www.publish.csiro.au/wf/WF16003


## Install

### Suggested procedure

Use fire-spread through Docker: 

    docker pull davidfrantz/fire-spread

### Install it yourself

    1) clone repository
    2) cd fire-spread
    3) make

Linux is required. GDAL is required.
The program will be installed in $HOME/bin

**If this fails, consider using docker instead.**


## Test data

A small testing dataset is included in this repository (testdata). 
You have to extract the data before using it.


## Usage

    fire-spread \
        input-stack input-dates output-dir basename \
        init-searchdist track-searchdist \
        temp-dist density-dist min-size smooth-dist \
        ncpu verbose


|Argument| Short Description|
|:---|:---|
|input-stack|input data (see below)|
|input-dates|list of input dates (see below)|
|output-dir|output directory|
|basename|basename, which is used for naming output|
|init-searchdist|see paper|
|track-searchdist|see paper|
|temp-dist|see paper|
|density-dist|see paper|
|min-size|see paper|
|smooth-dist|see paper|
|ncpu|number of CPUs|
|verbose|print detailed information (v) or be quiet (q)|

**Sample call**:

    docker run \
        -v ~/testdata:/data \
        -u $(id -u):$(id -g) \
        -t --rm \
        davidfrantz/fire-spread \
        fire-spread \
        /data/CCI_1M_DOY_2001-2020_LAEA_example.dat \
        /data/timeseries.txt \
        /data/output \
        example_esacci \
        10 10 5 12 2 3 10 v


## Input data

### input-stack

* monthly stack of burned area data (DOY)
* mosaic of complete area
* reprojected to some projection which minimizes distortion of both area and angles (e.g. equidistant, see https://doi.org/10.1016/j.cageo.2014.07.005)
* 16bit unsigned data format

### Hints

* We developed the code for the MODIS burned area product MCD45A1 collection 5.1 but used it also successfully for MCD64 collection 6 (https://modis-fire.umd.edu/guides.html) and the ESA CCI data versio) 5.1 (https://geogra.uah.es/fire_cci/firecci51.php) 
* The different fire seasons (one per year) will be automatically identified, and parallel processing (one CPU per season) will be used for speeding things up.

### input-dates

* text file, which contains the dates of the input-stack as YYYYMM
* each date in one line
* last line should be empty
* use Unix EOL


## A short note on the code

This is dirty research code from the early phase of my C-coding career ;)
Not polished, but it gets the job done.
