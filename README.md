# devEMF: EMF Graphics Output Device

devEMF is a [R](https://www.r-project.org) package that provides a graphics device driver that allows R to seamlessly produce EMF and EMF+ vector graphics.

[Enhanced Metafile (EMF)](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emf) and [EMF+](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus) were developed by Microsoft, published as open specifications, and are widely supported across platforms and office software suites.  EMF+ is a newer format that is encapsulated within an EMF file and allows options such as partial transparency.

See NEWS file for package history; code was transferred from subversion to git in Nov 2022 so git logs are incomplete.

#### Installation

Install as any R package from [CRAN](https://cran.r-project.org/web/packages/devEMF/index.html) using
```r
install.packages("devEMF")
```

If you want to install from this git repository, from the directory above this, first build an R package:
```
R CMD build devEMF
```
The resulting package will be named devEMF_XXX.tar.gz where XXX is replaced with the current R package version number found in the DESCRIPTION.  This file can then be installed as with any local R package installed from source.


#### Compiling

R will automatically compile the underlying C++ code during installation.  On Windows and MacOS there are no additional dependancies.  On other systems, some basic dependencies are required to access the font metric information necessary to place text.  If [FreeType](https://freetype.org/) or [Xft](https://www.freedesktop.org/wiki/Software/Xft/) are available (perhaps through installing an operating system package called libfreetype-dev or libxft-dev), then devEMF can access font metric information for any installed font; if neither FreeType nor Xft are available, devEMF will attempt to use [zlib](https://www.zlib.net/) to access font metric information from the 14 core Adobe fonts included in devEMF under inst/.

#### Bug reports
Developers or users reporting bugs/feature requests shoulds create an issue on [github](https://github.com/plfjohnson/devEMF).
