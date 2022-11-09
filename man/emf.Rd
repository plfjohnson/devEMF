\name{emf}
\Rdversion{1.1}
\alias{emf}
\alias{devEMF}
\title{Enhanced Metafile Graphics Device}
\description{
  'emf' starts the graphics device driver for producing enhanced
  metafile (emf+/emf) graphics, which can imported natively as vector
  graphics in both LibreOffice and Microsoft Office.
}
\usage{

emf(file = "Rplot.emf", width = 7, height = 7, units = c("in", "cm", "mm"),
    bg = "transparent", fg = "black", pointsize = 12,
    family = "Helvetica", coordDPI = 300, custom.lty=emfPlus,
    emfPlus=TRUE, emfPlusFont = FALSE, emfPlusRaster = FALSE,
    emfPlusFontToPath = FALSE)
}

\arguments{
  \item{file}{character string giving the name of file.}
  \item{width}{width of plot.}
  \item{height}{height of plot.}
  \item{units}{units in which \code{width} and \code{height} are
  given.  Must be one of \code{in} (inches, default), \code{cm}, or \code{mm}.}
  \item{bg}{plot background color.  If \code{transparent}, no background
    color is painted.}
  \item{fg}{initial foreground color to be used.}
  \item{pointsize}{default font point size to be used.}
  \item{family}{default font family to be used (if you use non-standard
  fonts, consider setting emfPlusFontToPath=TRUE to ensure text looks
  correct on the viewing system).}
  \item{coordDPI}{DPI used for coordinate system (while EMF is a vector
    format, it (and R) use discrete coordinate systems).}
  \item{custom.lty}{logical: if false, approximate R line types with EMF
    standard line types (any program that imports EMF should support
    this).  If true, use exact (custom) R line type, which is only
    supported by LibreOffice if using EMF+.}
  \item{emfPlus}{logical: whether to use EMF+ records or not}
  \item{emfPlusFont}{logical: if using EMF+, should text use EMF+ or EMF
    records?}
  \item{emfPlusRaster}{logical: if using EMF+, should raster images use
    EMF+ or EMF records?}
  \item{emfPlusFontToPath}{logical: if using EMF+, should text be
    converted to graphics paths and saved in file?}
}
\details{
  The standard office suites support very few vector graphics formats
  for import.  Enhanced Metafiles (EMFs) do tend to be supported, which
  is the purpose of this device.  EMF+ is a newer format that is
  encapsulated within an EMF file and allows options such as partial
  transparency.

  The EMF/EMF+ specifications do not allow for multiple pages; attempting to
  create multiple pages will result in a warning message and the output
  file will contain the different pages superimposed.
  
  Also, note EMF/EMF+ does not support font embedding, so with the
  default options, any fonts used must be present both on the system
  that you use when you create the graphics file (i.e., when you run R)
  and on the system that you use to import/view the file.  HOWEVER, the
  \code{emfPlusFontToPath = TRUE} option converts text characters into
  graphics paths and makes the resulting EMF+ files truly portable
  (although text strings will no longer be editable by downstream
  programs).  This option is currently only implemented for Linux (with
  FreeType and Xft installed) and Windows.  Contact the author if you'd
  like to request implementation on Apple.

  EMF/EMF+ supports Unicode characters, and this package tries to
  maintain that support as well.  However, font metric information is
  system dependent and on linux depends on FreeType or XFT being
  installed.  Regardless, basic font metrics for the standard Adobe
  PostScript font families are built into this package.

  Only EMF+ allows partial transparency (i.e., the only useful type --
  0.0 < alpha < 1.0); attempting to use a transparent color when
  \code{emfPlus = FALSE} will result in a warning message and the output
  will be completely transparent (invisible).

  LibreOffice support for EMF+ was incomplete as of version 5, and,
  cannot handle EMF+ records with rotated text or raster images (hence
  the options to turn off EMF+ and use EMF instead for these types of
  records).  devEMF defaults to EMF for these records to maintain
  compatibility, but quality is higher if EMF+ records are used.
}
\section{Known limitations}{
  \itemize{
    \item EMF (as opposed to EMF+) raster rendering does not support
  interpolation control.
    \item EMF (as opposed to EMF+) does not support an alpha channel.
    \item Path rendering is not implemented for EMF (as opposed to EMF+).
  }
}
\author{
  Philip Johnson
}
\seealso{
  \code{\link{Devices}}
}
\examples{
require(devEMF)
\dontrun{
# open file "bar.emf" for graphics output
emf("bar.emf")
# produce the desired graph(s)
plot(1,1)
dev.off() #turn off device and finalize file
}
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{device}
