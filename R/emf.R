#  Part of the devEMF Package for R.  Copyright 2011 by Philip Johnson.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  A copy of the GNU General Public License is available at
#  http://www.r-project.org/Licenses/

emf <- function(file = "Rplot.emf", width=7, height=7, units=c("in","cm","mm"),
                bg = "transparent", fg = "black", pointsize=12,
                family = "Helvetica", coordDPI = 300,
                custom.lty=emfPlus, emfPlus=TRUE,
                emfPlusFont = FALSE, emfPlusRaster = FALSE,
                emfPlusFontToPath = FALSE)
{
    if (is.na(width) ||  width < 0 ||  is.na(height)  ||  height < 0) {
        stop("emf: both width and height must be positive numbers.");
    }
    units = match.arg(units)
    if (units == "cm") {
        width = width / 2.54
        height = height / 2.54
    } else if (units == "mm") {
        width = width / 254
        height = height / 254
    }
    if (emfPlusFont  &&  emfPlusFontToPath) {
        stop("emf: at most one of 'emfPlusFont' and 'emfPlusFontToPath' can be TRUE")
    }
  .External(devEMF, file, bg, fg, width, height, pointsize,
            family, coordDPI, custom.lty, emfPlus, emfPlusFont, emfPlusRaster,
            emfPlusFontToPath)
  invisible()
}
