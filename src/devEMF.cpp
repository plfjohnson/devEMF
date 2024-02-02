/* $Id: devEMF.cpp 363 2022-06-23 15:04:46Z pjohnson $
    --------------------------------------------------------------------------
    Add-on package to R to produce EMF graphics output (for import as
    a high-quality vector graphic into Microsoft Office or OpenOffice).


    Copyright (C) 2011-2015 Philip Johnson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    --------------------------------------------------------------------------
*/

#include <Rconfig.h>

#define STRICT_R_HEADERS
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include <R_ext/GraphicsEngine.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Riconv.h>

#include <fstream>
#include <set>
#include <sstream>
//#include <iostream> // DEBUG ONLY
#include <map>

#include "emf.h"  //defines EMF data structures
#include "emf+.h" //defines EMF+ data structures
#include "fontmetrics.h" //platform-specific font metric code

using namespace std;


class CDevEMF {
public:
    CDevEMF(const char *defaultFontFamily, int coordDPI, bool customLty,
            bool emfPlus, bool emfpFont, bool emfpRaster, bool emfpEmbed) :
        m_debug(false) {
        m_DefaultFontFamily = defaultFontFamily;
        m_PageNum = 0;
        m_NumRecords = 0;
        m_CurrHadj = m_CurrPolyFill = -100;
        m_CurrClip[0] = m_CurrClip[1] = m_CurrClip[2] = m_CurrClip[3] = -1;
        m_CoordDPI = coordDPI;
        //feature options
        m_UseCustomLty = customLty;
        m_UseEMFPlus = emfPlus;
        if (m_debug) Rprintf("using emfplus: %d\n", emfPlus);
        m_UseEMFPlusFont = emfpFont;
        m_UseEMFPlusRaster = emfpRaster;
        m_UseEMFPlusTextToPath = emfpEmbed;
    }

    // Member-function R callbacks (see below class definition for
    // extern "C" versions
    bool Open(const char* filename, int width, int height);
    void Close(void);
    void NewPage(const pGEcontext gc);
    void MetricInfo(int c, const pGEcontext gc, double* ascent,
                    double* descent, double* width);
    double StrWidth(const char *str, const pGEcontext gc);
    void Clip(double x0, double x1, double y0, double y1);
    void Circle(double x, double y, double r, const pGEcontext gc);
    void Line(double x1, double y1, double x2, double y2, const pGEcontext gc);
    void Polyline(int n, double *x, double *y, const pGEcontext gc);
    void TextUTF8(double x, double y, const char *str, double rot,
                  double hadj, const pGEcontext gc);

    void Rect(double x0, double y0, double x1, double y1, const pGEcontext gc);
    void Polygon(int n, double *x, double *y, const pGEcontext gc);
    void Path(double *x, double *y, int nPoly, int *nPts, bool winding,
              const pGEcontext gc);
    void Raster(unsigned int* data, int w, int h, double x, double y,
                double width, double height, double rot,
                Rboolean interpolate);

    // helper functions
    int Inches2Dev(double inches) { return m_CoordDPI*inches;}
    static double x_EffPointsize(const pGEcontext gc) {
        return floor(gc->cex * gc->ps + 0.5);
    }

private:
    static string iConvUTF8toUTF16LE(const string& s) {
        void *cd = Riconv_open("UTF-16LE", "UTF-8");
        if (cd == (void*)(-1)) {
            Rf_error("EMF device failed to convert UTF-8 to UTF-16LE");
            return "";
        }
        size_t inLeft = s.length();
        size_t outLeft = s.length()*4;//overestimate
        char *utf16 = new char[outLeft];
        const char *in = s.c_str();
        char *out = utf16;
        size_t res = Riconv(cd, &in, &inLeft, &out, &outLeft);
        if (res != 0) {
            delete[] utf16;
            Rf_error("Text string not valid UTF-8.");
            return "";
        }
        string ret(utf16, s.length()*4 - outLeft);
        delete[] utf16;
        Riconv_close(cd);
        return ret;
    }
    void x_TransformY(double* y, int n) {
        for (int i = 0; i < n;  ++i, ++y) *y = m_Height - *y;
    }

    unsigned char x_GetPen(const pGEcontext gc) {
        return m_UseEMFPlus ?
            m_ObjectTable.GetPen(gc->col, gc->lwd*72./96., gc->lty, gc->lend,
                                 gc->ljoin, gc->lmitre, Inches2Dev(1)/72.,
                                 m_UseCustomLty, m_File) :
            m_ObjectTableEMF.GetPen(gc->col, gc->lwd*72./96., gc->lty, gc->lend,
                                    gc->ljoin, gc->lmitre, Inches2Dev(1)/72.,
                                    m_UseCustomLty, m_File);
    }
    int x_GetBrush(const pGEcontext gc) {
        if (!m_UseEMFPlus) {
            return (m_ObjectTableEMF.GetBrush(gc->fill, m_File));
        }
        bool hasFill = !R_TRANSPARENT(gc->fill);
#if R_GE_version >= 13
        hasFill = hasFill  ||  (gc->patternFill != R_NilValue);
#endif
        if (!hasFill) { // no brush needed!
            return -1;
        }
        if (!R_TRANSPARENT(gc->fill)) {
            return m_ObjectTable.GetBrush(new EMFPLUS::SBrush(gc->fill),m_File);
        }
#if R_GE_version >= 13
        switch (R_GE_patternType(gc->patternFill)) {
        case R_GE_linearGradientPattern: {
            EMFPLUS::SBrush* b =
                new EMFPLUS::SBrush(EMFPLUS::eBrushTypeLinearGradient);
            b->gradCoords.x = R_GE_linearGradientX1(gc->patternFill);
            b->gradCoords.y = R_GE_linearGradientY1(gc->patternFill);
            x_TransformY(&b->gradCoords.y, 1);
            b->gradCoords.w = R_GE_linearGradientX2(gc->patternFill) -
                b->gradCoords.x;
            double y2 = R_GE_linearGradientY2(gc->patternFill);
            x_TransformY(&y2, 1);
            b->gradCoords.h =  y2 - b->gradCoords.y;
            switch (R_GE_linearGradientExtend(gc->patternFill)) {
                //not sure if pad/none are correctly mapped..
            case R_GE_patternExtendPad:
                b->wrapMode = EMFPLUS::eWrapModeClamp; break;
            case R_GE_patternExtendRepeat:
                b->wrapMode = EMFPLUS::eWrapModeTile; break;
            case R_GE_patternExtendReflect:
                b->wrapMode = EMFPLUS::eWrapModeTileFlipXY; break;
            case R_GE_patternExtendNone:
                b->wrapMode = EMFPLUS::eWrapModeClamp; break;
            }
            int n = R_GE_linearGradientNumStops(gc->patternFill);
            b->blendVector.resize(n);
            for (int i = 0;  i < n;  ++i) {
                b->blendVector[i].pos =
                    R_GE_linearGradientStop(gc->patternFill, i);
                b->blendVector[i].col =
                    R_GE_linearGradientColour(gc->patternFill, i);
            }
            return m_ObjectTable.GetBrush(b, m_File);
        }
        default:
            Rf_warning("brush pattern type unsupported by devEMF");
        }
#endif
        return -1;
    }

    class CFontInfoIndex : public map<SSysFontInfo::SFontSpec, SSysFontInfo*> {
    public:
        ~CFontInfoIndex(void) {
            for (iterator i = begin();  i != end();  ++i) {
                delete i->second;
            }
        }
    };
    SSysFontInfo* x_GetFontInfo(const pGEcontext gc,
                                const char *fontfamily = NULL) {
        int face = (gc->fontface < 1  || gc->fontface > 4) ? 1 : gc->fontface;
        const char *family = fontfamily ? fontfamily :
            gc->fontfamily[0] != '\0' ? gc->fontfamily :
            m_DefaultFontFamily.c_str();
        SSysFontInfo::SFontSpec spec(family, face,
                                     Inches2Dev(x_EffPointsize(gc)/72));
        CFontInfoIndex::iterator i = m_FontInfoIndex.find(spec);
        if (i == m_FontInfoIndex.end()) {
            SSysFontInfo* info = new SSysFontInfo(spec);
            m_FontInfoIndex[spec] = info;
            return info;
        } else {
            return i->second;
        }
    }
    unsigned char x_GetFont(const pGEcontext gc, SSysFontInfo *info = NULL,
                            double rot = 0) {
        if (info == NULL) {
            info = x_GetFontInfo(gc);
        }
        return m_UseEMFPlus  &&  m_UseEMFPlusFont ?
            m_ObjectTable.GetFont(info->m_Spec.m_Face,
                                  info->m_Spec.m_Size,
                                  iConvUTF8toUTF16LE(info->m_Spec.m_Family),
                                  m_File) :
            m_ObjectTableEMF.GetFont(info->m_Spec.m_Face,
                                     info->m_Spec.m_Size,
                                     iConvUTF8toUTF16LE(info->m_Spec.m_Family),
                                     rot, m_File);
    }
    void x_SetEMFTextColor(int col) {
        EMF::S_SETTEXTCOLOR emr;
        emr.color.Set(R_RED(col), R_GREEN(col), R_BLUE(col));
        if (R_ALPHA(col) > 0  &&  R_ALPHA(col) < 255) {
            Rf_warning("partial transparency is not supported for EMF "
                       "fonts (consider enabling EMF+, although be aware "
                       "LibreOffice EMF+ font support is incomplete)");
        }
        emr.Write(m_File);
        m_CurrTextCol = col;
    }


private:
    bool m_debug;
    EMF::ofstream m_File;
    int m_NumRecords;
    int m_PageNum;
    int m_Width, m_Height;
    string m_DefaultFontFamily;
    int m_CoordDPI;
    bool m_UseCustomLty;
    bool m_UseEMFPlus;
    bool m_UseEMFPlusFont;
    bool m_UseEMFPlusRaster;
    bool m_UseEMFPlusTextToPath;

    //EMF states
    double m_CurrHadj;
    int m_CurrTextCol;
    int m_CurrPolyFill;
    double m_CurrClip[4];

    //EMF/EMF+ objects
    EMFPLUS::CObjectTable m_ObjectTable;
    EMF::CObjectTable m_ObjectTableEMF;

    //system info for font metrics
    CFontInfoIndex m_FontInfoIndex;
};

// R callbacks below (declare extern "C")
namespace EMFcb {
    extern "C" {
        void Activate(pDevDesc) {}
        void Deactivate(pDevDesc) {}
        void Mode(int, pDevDesc) {}
        Rboolean Locator(double*, double*, pDevDesc) {return FALSE;}
        void Raster(unsigned int* r, int w, int h, double x, double y,
                    double width, double height, double rot,
                    Rboolean interpolate, const pGEcontext, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->
                Raster(r,w,h,x,y,width,height,rot,interpolate);
        }
        SEXP Cap(pDevDesc) {
            Rf_warning("Raster capture not available for EMF");
            return R_NilValue;
        }
        void Path(double* x, double* y, int n, int* np, Rboolean wnd,
                  const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Path(x,y, n,np, wnd, gc);
        }
        void Close(pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Close();
            delete static_cast<CDevEMF*>(dd->deviceSpecific);
        }
        void NewPage(const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->NewPage(gc);
        }
        void MetricInfo(int c, const pGEcontext gc, double* ascent,
                        double* descent, double* width, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->
                MetricInfo(c, gc, ascent,descent, width);
        }
        double StrWidth(const char *str, const pGEcontext gc, pDevDesc dd) {
            return static_cast<CDevEMF*>(dd->deviceSpecific)->StrWidth(str, gc);
        }
        void Clip(double x0, double x1, double y0, double y1, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Clip(x0,x1,y0,y1);
        }
        void Circle(double x, double y, double r, const pGEcontext gc,
                    pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Circle(x,y,r,gc);
        }
        void Line(double x1, double y1, double x2, double y2,
                  const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Line(x1,y1,x2,y2,gc);
        }
        void EMFcb_Polyline(int n, double *x, double *y,
                      const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Polyline(n,x,y, gc);
        }
        void TextUTF8(double x, double y, const char *str, double rot,
                      double hadj, const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->
                TextUTF8(x,y,str,rot,hadj,gc);
        }
        void Text(double, double, const char *, double,
                  double, const pGEcontext, pDevDesc) {
            Rf_warning("Non-UTF8 text currently unsupported in devEMF.");
        }
        void Rect(double x0, double y0, double x1, double y1,
                  const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Rect(x0,y0,x1,y1,gc);
        }
        void EMFcb_Polygon(int n, double *x, double *y,
                     const pGEcontext gc, pDevDesc dd) {
            static_cast<CDevEMF*>(dd->deviceSpecific)->Polygon(n,x,y,gc);
        }

        void Size(double *left, double *right, double *bottom, double *top,
                  pDevDesc dd) {
            *left = dd->left; *right = dd->right;
            *bottom = dd->bottom; *top = dd->top;
        }
        SEXP setPattern(SEXP pattern, pDevDesc) {
            return pattern;
        }
        void releasePattern(SEXP, pDevDesc) {} 

        //unimplemented stubs (these additions to the R graphics
        //engine appear primarily targeted at Cairo graphics
        SEXP setClipPath(SEXP, SEXP, pDevDesc) { return R_NilValue; }
        void releaseClipPath(SEXP, pDevDesc) {}
        SEXP setMask(SEXP, SEXP, pDevDesc) {return R_NilValue;}
        void releaseMask(SEXP, pDevDesc) {}
    }
} //end of R callbacks / extern "C"


void CDevEMF::MetricInfo(int c, const pGEcontext gc, double* ascent,
                     double* descent, double* width)
{
    if (m_debug) Rprintf("metricinfo: %c %i %x (face %i, pts %f)\n",c,c,abs(c),gc->fontface, x_EffPointsize(gc));
    //cout << gc->fontfamily << "/" << gc->fontface << " -- " << c << " / " << (char) c;
    if (c < 0) { c = -c; }

    SSysFontInfo *info;
    info = x_GetFontInfo(gc);
    if (!(info  &&  info->HasChar(c))  &&  gc->fontface == 5) {
        // check for char existence in "Symbol" font
        info = x_GetFontInfo(gc, "Symbol");
    } else if (!info) {
        //last ditch attempt
        info = x_GetFontInfo(gc, "sans");
        if (info) {
            Rf_warning("devEMF: using 'sans' font metrics instead of requested '%s'",
                       gc->fontfamily);
        }
    }
    if (!info) { //no metrics available
        *ascent = 0;
        *descent = 0;
        *width = 0;
    } else {
        info->GetMetrics(c, *ascent, *descent, *width);
    }
    if (m_debug) Rprintf("\t%f/%f/%f\n",*ascent,*descent,*width);
}


double CDevEMF::StrWidth(const char *str, const pGEcontext gc) {
    if (m_debug) Rprintf("strwidth ('%s') --> ", str);

    SSysFontInfo *info = x_GetFontInfo(gc);
    double width =  info ? info->GetStrWidth(str) : 0;

    if (m_debug) Rprintf("%f\n", width);
    //cout << "strwidth: " << width/Inches2Dev(1) << endl;
    return width;
}

	/* Initialize the device */

bool CDevEMF::Open(const char* filename, int width, int height)
{
    if (m_debug) Rprintf("open: %i, %i\n", width, height);
    m_Width = width;
    m_Height = height;
    
    m_File.open(R_ExpandFileName(filename), ios_base::binary);
    if (!m_File) {
	return FALSE;
    }

    {
        EMF::SHeader emr;
        emr.bounds.Set(0,0,m_Width,m_Height); //device units
        emr.frame.Set(0,0, // units of 0.01mm
                      m_Width * (2540./Inches2Dev(1)),
                      m_Height * (2540./Inches2Dev(1)));
        emr.signature = 0x464D4520;
        emr.version = 0x00010000;
        emr.nBytes = 0;   //WILL EDIT WHEN CLOSING
        emr.nRecords = 0; //WILL EDIT WHEN CLOSING
        emr.nHandles = 0; //WILL EDIT WHEN CLOSING
        emr.reserved = 0x0000;
        string ver = "?"; // devEMF version
        {//find version using R "packageVersion" function
            SEXP packageVer, call, res;
            PROTECT(packageVer = Rf_findFun(Rf_install("packageVersion"),
                                            R_GlobalEnv));
            PROTECT(call = Rf_lang2(packageVer, Rf_ScalarString
                                    (Rf_mkChar("devEMF"))));
            PROTECT(res = Rf_eval(call, R_GlobalEnv));
            if (Rf_isVector(res)  &&  Rf_length(res) == 1  &&
                Rf_isInteger(VECTOR_ELT(res,0))  &&
                Rf_length(VECTOR_ELT(res,0)) >= 1) {
                std::ostringstream oss;
                oss << INTEGER(VECTOR_ELT(res,0))[0];
                if (Rf_length(VECTOR_ELT(res,0)) >= 2) {
                    oss << "." << INTEGER(VECTOR_ELT(res,0))[1];
                    if (Rf_length(VECTOR_ELT(res,0)) >= 3) {
                        oss << "." << INTEGER(VECTOR_ELT(res,0))[2];
                    }
                }
                ver = oss.str();
            }
            UNPROTECT(3);
        }
        //Description string must be UTF-16LE
        emr.desc = iConvUTF8toUTF16LE("Created by R using devEMF ver. "+ver);
        emr.nDescription = emr.desc.length()/2;
        emr.offDescription = 0; //set during serialization
        emr.nPalEntries = 0;
        emr.device.Set(m_Width, m_Height);
        emr.millimeters.Set(m_Width * (25.4/Inches2Dev(1)) + 0.5, //round
                            m_Height * (25.4/Inches2Dev(1)) + 0.5); //round
        emr.cbPixelFormat=0x00000000;
        emr.offPixelFormat=0x00000000;
        emr.bOpenGL=0x00000000;
        emr.micrometers.Set(m_Width * (25400./Inches2Dev(1)),
                            m_Height * (25400./Inches2Dev(1)));
        emr.Write(m_File);
    }

    if (m_UseEMFPlus) {
        {
            EMFPLUS::SHeader emr;
            emr.plusFlags = 0; //specifies for video display context
            emr.dpiX = Inches2Dev(1);
            emr.dpiY = Inches2Dev(1);
            emr.Write(m_File);
        }
        { // page transform (1 pixel == 1 device unit)
            EMFPLUS::SSetPageTransform emr(EMFPLUS::eUnitPixel, 1);
            emr.Write(m_File);
        }
        {// use decent anti-aliasing (key for viewing in MS Office)
            EMFPLUS::SSetAntiAliasMode emr(true);
            emr.Write(m_File);
        }
        {// (unclear if affects anything with either MS or Libre Office)
            EMFPLUS::SSetTextRenderingHint emr(EMFPLUS::eTRHAntiAliasGridFit);
            emr.Write(m_File);
        }
        {// ask for precise coordinates (reduces thin gaps between
         // nominally adjacent drawn objects as well as other unwanted
         // behavior when drawing raster images)
            EMFPLUS::SSetPixelOffsetMode emr(EMFPLUS::ePixelOffsetModeHalf);
            emr.Write(m_File);
        }
    }

    if (!m_UseEMFPlus  ||  !m_UseEMFPlusFont  ||  !m_UseEMFPlusRaster) {
        {// mapping mode (1 logical unit == 1 device unit)
            EMF::S_SETMAPMODE emr;
            emr.mode = EMF::eMM_TEXT;
            emr.Write(m_File);
        }
        
        {//Don't paint text box background
            EMF::S_SETBKMODE emr;
            emr.mode = EMF::eTRANSPARENT;
            emr.Write(m_File);
        }

        //Initialize text color
        x_SetEMFTextColor(R_RGBA(0,0,0,255));
    }
    return TRUE;
}

void CDevEMF::NewPage(const pGEcontext gc) {
    if (++m_PageNum > 1) {
        Rf_warning("Multiple pages not available for EMF device");
    }
    if (R_OPAQUE(gc->fill)) {
	gc->col = R_TRANWHITE; // no line around border
        Rect(0, 0, m_Width, m_Height, gc);
    }
}


void CDevEMF::Clip(double x0, double x1, double y0, double y1)
{
    if (m_debug) Rprintf("clip %f,%f,%f,%f\n", x0,y0,x1,y1);
    if ((m_CurrClip[0] == x0  &&
         m_CurrClip[1] == y0  &&
         m_CurrClip[2] == x1  &&
         m_CurrClip[3] == y1  &&
         m_CurrClip[0] != -1  &&
         m_CurrClip[1] != -1  &&
         m_CurrClip[2] != -1  &&
         m_CurrClip[3] != -1)) {
        return; //skip if unchanged
    }
    m_CurrClip[0] = x0;
    m_CurrClip[1] = y0;
    m_CurrClip[2] = x1;
    m_CurrClip[3] = y1;
    x_TransformY(&y0, 1);
    x_TransformY(&y1, 1);
    if (m_UseEMFPlus) {
        EMFPLUS::SSetClipRect clip(EMFPLUS::eCombineModeReplace, x0,y0,x1,y1);
        clip.Write(m_File);
    }
    //also do EMF clipping if possibly using any EMF records
    if (!m_UseEMFPlus  ||  !m_UseEMFPlusFont  ||  !m_UseEMFPlusRaster)  {
        EMF::S_EXTSELECTCLIPRGN rgn;//reset to default
        rgn.Write(m_File);
        EMF::S_INTERSECTCLIPRECT rect(x0,y0,x1,y1);
        rect.Write(m_File);
    }
    return;
}


void CDevEMF::Close(void)
{
    if (m_debug) Rprintf("close\n");

    if (m_UseEMFPlus) {
        EMFPLUS::SEndOfFile empr;
        empr.Write(m_File);
    }

    { //EOF record
        EMF::S_EOF emr;
        emr.nPalEntries = 0;
        emr.offPalEntries = 0;
        emr.nSizeLast = sizeof(emr);
        emr.Write(m_File);
    }
    

    { //Edit header record to report number of records, handles & size
        unsigned int nBytes = m_File.tellp();
        m_File.seekp(4*12);//offset of nBytes field of EMF header
        string data;
        data << EMF::TUInt4(nBytes)
             << EMF::TUInt4(m_File.nRecords)
            //not mentioned in spec, but seems to need one extra handle
             << EMF::TUInt4(m_ObjectTableEMF.GetSize()+1);
        m_File.write(data.data(), 12);
        m_File.close();
    }
}

void CDevEMF::Raster(unsigned int* r, int w, int h, double x, double y,
                     double width, double height, double rot,
                     Rboolean interpolate) {
    if (m_debug) Rprintf("raster: %d,%d / %f,%f,%f,%f\n", w,h,x,y,width,height);
    
    x_TransformY(&y, 1);//EMF has origin in upper left; R in lower left
    y -= height;
    /* Sigh.. as of 2016, LibreOffice support for EMF+ raster ops is broken/missing .*/
    if (m_UseEMFPlus  &&  m_UseEMFPlusRaster) {
        if (rot != 0) {
            EMFPLUS::SMultiplyWorldTransform trans
                (cos(rot*M_PI/180), -sin(rot*M_PI/180),
                 sin(rot*M_PI/180), cos(rot*M_PI/180),
                 x, y+height);
            trans.Write(m_File);
            x = 0; y = -height; //rotate around ll corner
        }
        EMFPLUS::SSetInterpolationMode m1
            (interpolate ?
             EMFPLUS::eInterpolationModeHighQualityBilinear:
             EMFPLUS::eInterpolationModeNearestNeighbor);
        m1.Write(m_File);
        EMFPLUS::SDrawImage image(m_ObjectTable.GetImage(r,w,h,m_File), w,h,
                                  x, y, width, height);
        image.Write(m_File);
        if (rot != 0) {
            EMFPLUS::SResetWorldTransform trans;
            trans.Write(m_File);
        }
    } else {
        /* Unfortunately, I can't figure out a interpolation control
           for EMF -- so this seems to leave it up to the client
           program (generally seems sort of linear interpolation) The
           following lines look like they might work.. but don't.
           Also tried with BITBLT instead of STRETCHBLT.  No luck either.
        if (!interpolate) {
           EMF::S_SETSTRETCHBLTMODE m1(3);
           m1.Write(m_File);
        }
        */
        if (rot != 0) {
            EMF::S_SETWORLDTRANSFORM emr;
            emr.xform.Set(cos(rot*M_PI/180), -sin(rot*M_PI/180),
                          sin(rot*M_PI/180), cos(rot*M_PI/180),
                          x, y+height);
            emr.Write(m_File);
            x = 0; y = -height; //rotate around ll corner
        }
        EMF::S_STRETCHBLT bmp(r, w,h,x,y,width,height);
        bmp.Write(m_File);
        if (rot != 0) {
            EMF::S_SETWORLDTRANSFORM emr;
            emr.xform.Set(1,0,0,1, 0,0);
            emr.Write(m_File);
        }
    }
}


void CDevEMF::Line(double x1, double y1, double x2, double y2,
	       const pGEcontext gc)
{
    if (m_debug) Rprintf("line\n");

    if (x1 != x2  ||  y1 != y2) {
	double x[2], y[2];
        x[0] = x1; x[1] = x2;
        y[0] = y1; y[1] = y2;
        Polyline(2, x, y, gc);
    }
}

void CDevEMF::Polyline(int n, double *x, double *y, const pGEcontext gc)
{
    if (m_debug) Rprintf("polyline\n");

    x_TransformY(y, n);//EMF has origin in upper left; R in lower left
    if (m_UseEMFPlus) {
        EMFPLUS::SDrawLines lines(n, x, y, x_GetPen(gc));
        lines.Write(m_File);
    } else {
        x_GetPen(gc);
        EMF::SPoly polyline(EMF::eEMR_POLYLINE, n, x, y);
        polyline.Write(m_File);
    }
}

void CDevEMF::Rect(double x0, double y0, double x1, double y1, const pGEcontext gc)
{
    if (m_debug) Rprintf("rect (converted to poly)\n");

    double x[4], y[4];
    x[0] = x[1] = x0;
    x[2] = x[3] = x1;
    y[0] = y[3] = y0;
    y[1] = y[2] = y1;
    Polygon(4, x, y, gc);
}

void CDevEMF::Circle(double x, double y, double r, const pGEcontext gc)
{
    if (m_debug) Rprintf("circle (%f,%f r=%f)\n", x, y,r);

    x_TransformY(&y, 1);//EMF has origin in upper left; R in lower left
    if (m_UseEMFPlus) {
        {
            EMFPLUS::SDrawEllipse circle(x-r, y-r, 2*r, 2*r, x_GetPen(gc));
            circle.Write(m_File);
        }
        int brushId = x_GetBrush(gc);
        if (brushId >= 0) {//not transparent
            EMFPLUS::SFillEllipse circle(x-r, y-r, 2*r, 2*r, brushId);
            circle.Write(m_File);
        }
    } else {
        x_GetPen(gc);
        x_GetBrush(gc);
        EMF::S_ELLIPSE emr;
        emr.box.Set(floor(x-r + 0.5), floor(y-r + 0.5),
                    floor(x+r + 0.5), floor(y+r + 0.5));
        emr.Write(m_File);
    }
}

void CDevEMF::Polygon(int n, double *x, double *y, const pGEcontext gc)
{
    if (m_debug) { Rprintf("polygon"); for (int i = 0; i<n;  ++i) {Rprintf("(%f,%f) ", x[i], y[i]);}; Rprintf("\n");}

    x_TransformY(y, n);//EMF has origin in upper left; R in lower left
    if (m_UseEMFPlus) {
        int pathId = m_ObjectTable.GetPath(new EMFPLUS::SPath(1,x,y,&n),m_File);
        int brushId = x_GetBrush(gc);
        if (brushId >= 0) {//not transparent
            EMFPLUS::SFillPath fill(pathId, brushId);
            fill.Write(m_File);
        }
        if (!R_TRANSPARENT(gc->col)) {
            EMFPLUS::SDrawPath drawPath(pathId, x_GetPen(gc));
            drawPath.Write(m_File);
        }
    } else {
        x_GetPen(gc);
        x_GetBrush(gc);
        EMF::SPoly polygon(EMF::eEMR_POLYGON, n, x, y);
        polygon.Write(m_File);
    }
}

void CDevEMF::Path(double *x, double *y, int nPoly, int *nPts, bool winding,
                   const pGEcontext gc)
{
    if (m_debug) { Rprintf("path\t(%d subpaths w/ %i winding)", nPoly, winding?1:0); }

    int n = 0;
    for (int i = 0;  i < nPoly;  ++i) {
        n += nPts[i];
    }
    x_TransformY(y, n);//EMF has origin in upper left; R in lower left

    if (m_UseEMFPlus) {
        // I can't find a way to make use of "winding" in EMF+
        int pathId = m_ObjectTable.GetPath(new EMFPLUS::SPath(nPoly,x,y,nPts),
                                           m_File);
        EMFPLUS::SDrawPath drawPath(pathId, x_GetPen(gc));
        drawPath.Write(m_File);
        int brushId = x_GetBrush(gc);
        if (brushId >= 0) {
            EMFPLUS::SFillPath fill(pathId, brushId);
            fill.Write(m_File);
        }
    } else {
        Rf_warning("devEMF does not implement 'path' drawing for EMF (only EMF+)");
        /*
        if (( winding  &&  m_CurrPolyFill != EMF::ePF_WINDING)  ||
            (!winding  &&  m_CurrPolyFill != EMF::ePF_ALTERNATE)) {
            m_CurrPolyFill = (winding) ? EMF::ePF_WINDING : EMF::ePF_ALTERNATE;
            EMF::S_SETPOLYFILLMODE emr;
            emr.mode = m_CurrPolyFill;
            emr.Write(m_File);
        }
        */
    }
}

void CDevEMF::TextUTF8(double x, double y, const char *str, double rot,
                       double hadj, const pGEcontext gc)
{
    if (m_debug) Rprintf("textUTF8: %s, %x  at %.1f %.1f\n", str, gc->col, x, y);
    x_TransformY(&y, 1);//EMF has origin in upper left; R in lower left

    SSysFontInfo *info = x_GetFontInfo(gc);
    if (m_UseEMFPlus  &&  m_UseEMFPlusTextToPath) { // pseudo-embed fonts
        //rotate & translate
        EMFPLUS::SMultiplyWorldTransform trans
            (cos(rot*M_PI/180), -sin(rot*M_PI/180),
             sin(rot*M_PI/180), cos(rot*M_PI/180),
             x, y);
        trans.Write(m_File);
        EMFPLUS::STranslateWorldTransform startAlign
            (-hadj*info->GetStrWidth(str), 0);
        startAlign.Write(m_File);

        //draw string -- have to convert UTF8 to UTF32
        unsigned int length = strlen(str);
        unsigned char len1, len2;
        unsigned long ch1, ch2;
        ch2 = SSysFontInfo::UTF8toUTF32(str, &len2);
        for (unsigned int i = 0;  i < length;  i += len1) {
            len1 = len2; ch1 = ch2;
            EMFPLUS::SPath *path = new EMFPLUS::SPath;
            info->AppendGlyphPath(ch1, *path);
            int pathId = m_ObjectTable.GetPath(path, m_File);
            EMFPLUS::SFillPath fill(pathId, R_RED(gc->col), R_GREEN(gc->col),
                                    R_BLUE(gc->col), R_ALPHA(gc->col));
            fill.Write(m_File);
            if (i + len1 < length) {
                ch2 = SSysFontInfo::UTF8toUTF32(str+i+len1, &len2);
                EMFPLUS::STranslateWorldTransform
                    advance(info->GetAdvance(ch1, ch2), 0);
                advance.Write(m_File);
            }
        }

        //reset rotation
        EMFPLUS::SResetWorldTransform reset;
        reset.Write(m_File);
        
    } else if (m_UseEMFPlus  &&  m_UseEMFPlusFont) { //Use EMF+ fonts
        if (rot != 0) {
            EMFPLUS::SMultiplyWorldTransform trans
                (cos(rot*M_PI/180), -sin(rot*M_PI/180),
                 sin(rot*M_PI/180), cos(rot*M_PI/180),
                 x, y);
            trans.Write(m_File);
            x = 0; y = 0; //because already translated!
        }
        EMFPLUS::SDrawString text
            (iConvUTF8toUTF16LE(str), gc->col, x_GetFont(gc, info),
             m_ObjectTable.GetStringFormat(hadj < 0.5 ? EMFPLUS::eStrAlignNear:
                                           (hadj==0.5 ? EMFPLUS::eStrAlignCenter:
                                            EMFPLUS::eStrAlignFar),
                                           EMFPLUS::eStrAlignNear, m_File));
        if (hadj == 0  ||  hadj == 0.5  ||  hadj == 1) {
            //already taken care of by request to align near/far
            text.m_LayoutRect.x = x;
        } else {
            text.m_LayoutRect.x = x + (hadj<0.5 ? -hadj : 1-hadj)*info->GetStrWidth(str);
        }
        double width, ascent, descent;
        info->GetFontBBox(ascent, descent, width);
        if (m_debug) Rprintf("fbbox: %.1f %.1f %.1f\n", ascent, descent, width);
        text.m_LayoutRect.y = y - ascent;//find baseline
        text.Write(m_File);
        if (rot != 0) {
            EMFPLUS::SResetWorldTransform trans;
            trans.Write(m_File);
        }
    } else { //otherwise EMF fonts
        x_GetFont(gc, info, rot);//inserts & selects font
        /* Commented out because Using rotation built into EMF font support; not as elegant but better supported by viewing/editing programs.
        if (rot != 0) {
            EMF::S_SETWORLDTRANSFORM emr;
            emr.xform.Set(cos(rot*M_PI/180), -sin(rot*M_PI/180),
                          sin(rot*M_PI/180), cos(rot*M_PI/180),
                          x, y);
            emr.Write(m_File);
            x = 0; y = 0; //because already translated!
        }
        */

        if (m_CurrTextCol != gc->col) {
            x_SetEMFTextColor(gc->col);
        }
        if (m_CurrHadj != hadj) {
            EMF::S_SETTEXTALIGN emr;
            emr.mode = (hadj < 0.5) ?
                EMF::eTA_BASELINE|EMF::eTA_LEFT :
                (hadj == 0.5 ? EMF::eTA_BASELINE|EMF::eTA_CENTER :
                 EMF::eTA_BASELINE|EMF::eTA_RIGHT);
            emr.Write(m_File);
            m_CurrHadj = hadj;
        }

        EMF::S_EXTTEXTOUTW emr;
        emr.bounds.Set(0,0,0,0);//EMF spec says to ignore
        emr.graphicsMode = EMF::eGM_COMPATIBLE;
        emr.exScale = 1;
        emr.eyScale = 1;
        if (hadj == 0  ||  hadj == 0.5  ||  hadj == 1) {
            //already taken care of by request to align left/center/right
            emr.emrtext.reference.Set(x,y);
        } else {
            double textWidth =  info ? info->GetStrWidth(str) : 0;
            if (hadj < 0.5) {
                emr.emrtext.reference.Set(x-floor(cos(rot*M_PI/180)*textWidth*hadj + 0.5),
                                          y+floor(sin(rot*M_PI/180)*textWidth*hadj + 0.5));
            } else {
                emr.emrtext.reference.Set(x+floor(cos(rot*M_PI/180)*textWidth*(1-hadj) + 0.5),
                                          y-floor(sin(rot*M_PI/180)*textWidth*(1-hadj) + 0.5));
            }
        }
        emr.emrtext.options = 0; // from spec, seems should be eETO_NO_RECT, but office does not seem to support this
        emr.emrtext.rect.Set(0,0,0,0);
        emr.emrtext.str = iConvUTF8toUTF16LE(str);
        emr.emrtext.nChars = emr.emrtext.str.length()/2;//spec says number of characters, but both Word & LibreOffice implement #bytes/2 (i.e., they don't collapse unicode supplemental planes that require multiple surrogates)
        // Below, calculate intercharacter spacing (spec implies this is optional, but Office 365 has difficulty exporting to pdf if missing and with text strings >=40 characters)
        unsigned int length = strlen(str);
        unsigned char len;
        unsigned long prevCh, nextCh;
        nextCh = SSysFontInfo::UTF8toUTF32(str, &len);
        for (unsigned int i = len;  i < length;  i += len) {
            prevCh = nextCh;
            nextCh = SSysFontInfo::UTF8toUTF32(str+i, &len);
            emr.emrtext.dx.push_back(info->GetAdvance(prevCh, nextCh));
        }
        //spec wants # advances = # characters, but maybe last is unused?
        emr.emrtext.dx.push_back(info->GetAdvance(nextCh, nextCh));

        emr.Write(m_File);
        /* Commented out for same reason as above
        if (rot != 0) {
            EMF::S_SETWORLDTRANSFORM emr;
            emr.xform.Set(1,0,0,1, 0,0);
            emr.Write(m_File);
        }
        */
    }
}


static
Rboolean EMFDeviceDriver(pDevDesc dd, const char *filename, 
                         const char *bg, const char *fg,
                         double width, double height, double pointsize,
                         const char *family, int coordDPI, bool customLty,
                         bool emfPlus, bool emfpFont, bool emfpRaster,
                         bool emfpEmbed)
{
    CDevEMF *emf;

    if (!(emf = new CDevEMF(family, coordDPI, customLty, emfPlus, emfpFont,
                            emfpRaster, emfpEmbed))){
	return FALSE;
    }
    dd->deviceSpecific = (void *) emf;

    dd->startfill = R_GE_str2col(bg);
    dd->startcol = R_GE_str2col(fg);
    dd->startps = floor(pointsize);//floor to maintain compat. w/ devPS.c
    dd->startlty = 0;
    dd->startfont = 1;
    dd->startgamma = 1;

    /* Device callbacks */
    dd->activate = EMFcb::Activate;
    dd->deactivate = EMFcb::Deactivate;
    dd->close = EMFcb::Close;
    dd->clip = EMFcb::Clip;
    dd->size = EMFcb::Size;
    dd->newPage = EMFcb::NewPage;
    dd->line = EMFcb::Line;
    dd->text = EMFcb::Text;
    dd->strWidth = EMFcb::StrWidth;
    dd->rect = EMFcb::Rect;
    dd->circle = EMFcb::Circle;
#if R_GE_version >= 6
    dd->raster = EMFcb::Raster;
    dd->cap = EMFcb::Cap;
#endif
#if R_GE_version >= 8
    dd->path = EMFcb::Path;
#endif
    dd->polygon = EMFcb::EMFcb_Polygon;
    dd->polyline = EMFcb::EMFcb_Polyline;
    dd->locator = EMFcb::Locator;
    dd->mode = EMFcb::Mode;
    dd->metricInfo = EMFcb::MetricInfo;
    dd->hasTextUTF8 = TRUE;
    dd->textUTF8       = EMFcb::TextUTF8;
    dd->strWidthUTF8   = EMFcb::StrWidth;
    dd->wantSymbolUTF8 = TRUE;
    dd->useRotatedTextInContour = TRUE;
    dd->canClip = TRUE;
    dd->canHAdj = 1;
    dd->canChangeGamma = FALSE;
    dd->displayListOn = FALSE;
#if R_GE_version >= 13
    dd->setPattern      = EMFcb::setPattern;
    dd->releasePattern  = EMFcb::releasePattern;
    dd->setClipPath     = EMFcb::setClipPath;
    dd->releaseClipPath = EMFcb::releaseClipPath;
    dd->setMask         = EMFcb::setMask;
    dd->releaseMask     = EMFcb::releaseMask;
#endif

    /* Screen Dimensions in device coordinates */
    dd->left = 0;
    dd->right = emf->Inches2Dev(width);
    dd->bottom = 0;
    dd->top = emf->Inches2Dev(height);

    /* Base Pointsize */
    /* Nominal Character Sizes in device units */
    dd->cra[0] = emf->Inches2Dev(0.9 * pointsize/72);
    dd->cra[1] = emf->Inches2Dev(1.2 * pointsize/72);

    /* Character Addressing Offsets */
    /* These offsets should center a single */
    /* plotting character over the plotting point. */
    /* Pure guesswork and eyeballing ... */
    dd->xCharOffset =  0.4900;
    dd->yCharOffset =  0.3333;
    dd->yLineBias = 0.2;

    /* Inches per device unit */
    dd->ipr[0] = dd->ipr[1] = 1./emf->Inches2Dev(1);

#if R_GE_version >= 13
    dd->deviceVersion = R_GE_definitions;
#endif

    if (!emf->Open(filename, dd->right, dd->top)) 
	return FALSE;

    return TRUE;
}

/*  EMF Device Driver Parameters
 *  --------------------
 *  file    = output filename
 *  bg	    = background color
 *  fg	    = foreground color
 *  width   = width in inches
 *  height  = height in inches
 *  pointsize = default font size in points
 *  family  = default font family
 *  userLty = whether to use custom ("user") line textures
 *  emfPlus = whether to use EMF+ format
 *  emfpFont = whether to use EMF+ text records
 *  emfpRaster = whether to use EMF+ raster records
 */
extern "C" {
SEXP devEMF(SEXP args)
{
    pGEDevDesc dd;
    const char *file, *bg, *fg, *family;
    double height, width, pointsize;
    Rboolean userLty, emfPlus, emfpFont, emfpRaster, emfpEmbed;
    int coordDPI;

    args = CDR(args); /* skip entry point name */
    file = Rf_translateChar(Rf_asChar(CAR(args))); args = CDR(args);
    bg = CHAR(Rf_asChar(CAR(args)));   args = CDR(args);
    fg = CHAR(Rf_asChar(CAR(args)));   args = CDR(args);
    width = Rf_asReal(CAR(args));	     args = CDR(args);
    height = Rf_asReal(CAR(args));	     args = CDR(args);
    pointsize = Rf_asReal(CAR(args));	     args = CDR(args);
    family = CHAR(Rf_asChar(CAR(args)));     args = CDR(args);
    coordDPI = Rf_asInteger(CAR(args));     args = CDR(args);
    userLty = (Rboolean) Rf_asLogical(CAR(args));     args = CDR(args);
    emfPlus = (Rboolean) Rf_asLogical(CAR(args));     args = CDR(args);
    emfpFont = (Rboolean) Rf_asLogical(CAR(args));     args = CDR(args);
    emfpRaster = (Rboolean) Rf_asLogical(CAR(args));     args = CDR(args);
    emfpEmbed = (Rboolean) Rf_asLogical(CAR(args));     args = CDR(args);

    R_GE_checkVersionOrDie(R_GE_version);
    R_CheckDeviceAvailable();
    BEGIN_SUSPEND_INTERRUPTS {
	pDevDesc dev;
	if (!(dev = (pDevDesc) calloc(1, sizeof(DevDesc))))
	    return 0;
	if(!EMFDeviceDriver(dev, file, bg, fg, width, height, pointsize,
                            family, coordDPI, userLty, emfPlus, emfpFont,
                            emfpRaster, emfpEmbed)) {
	    free(dev);
	    Rf_error("unable to start %s() device", "emf");
	}
	dd = GEcreateDevDesc(dev);
	GEaddDevice2(dd, "emf");
    } END_SUSPEND_INTERRUPTS;
    return R_NilValue;
}

    const R_ExternalMethodDef ExtEntries[] = {
        {"devEMF", (DL_FUNC)&devEMF, 13},
	{NULL, NULL, 0}
    };
    void R_init_devEMF(DllInfo *dll) {
	R_registerRoutines(dll, NULL, NULL, NULL, ExtEntries);
	R_useDynamicSymbols(dll, FALSE);
    }

} //end extern "C"
