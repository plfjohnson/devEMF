/* $Id$
    --------------------------------------------------------------------------
    Add-on package to R to produce EMF graphics output (for import as
    a high-quality vector graphic into Microsoft Office or OpenOffice).


    Copyright (C) 2011,2015 Philip Johnson

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


    Note this header file is C++ (R policy requires that all headers
    end with .h).

    This header contains platform-specific code for accessing font
    metric information directly from the windowing system.
    --------------------------------------------------------------------------
*/

/**** *nix headers ****/
#ifndef __APPLE__
#ifndef WIN32
#ifdef HAVE_ZLIB
#include <zlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#endif
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

#endif /* end not windows */
#endif /* end not mac */

/**** windows headers ****/
#ifdef WIN32
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef FALSE //sigh.. windows #defs are a pain
#undef TRUE  //sigh.. windows #defs are a pain
#include <map>
#endif /* end WIN32 */

/**** apple headers ****/
#ifdef __APPLE__
#include <CoreText/CTFont.h>
#include <CoreText/CTFontDescriptor.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>
#undef TRUE //sigh..it's not just windows
#undef FALSE
#endif /* end __APPLE__ */

/****************************************************************************/
// First make definitions common to all three systems
// then further below split apart system-specific code

namespace EMFPLUS { //forward declaration to avoid circularity..
struct SPath;
}

struct SSysFontInfo {
    struct SFontSpec {
        std::string m_Family;
        unsigned int m_Face;
        unsigned int m_Size;
        SFontSpec(const std::string &fam, int face, int size) :
            m_Family(fam), m_Face(face), m_Size(size) {
            if (face < 1  ||  face > 4) {
                Rf_error("Invalid font face requested");
            }
        }
        friend bool operator< (const SFontSpec &s1, const SFontSpec &s2) {
            int cmp = s1.m_Family.compare(s1.m_Family);
            if (cmp < 0) {
                return true;
            }
            return (cmp == 0  &&
                    (s1.m_Face < s2.m_Face  ||
                     (s1.m_Face == s2.m_Face  &&  s1.m_Size < s2.m_Size)));
        }
    };
    SFontSpec m_Spec;

    static unsigned char UTF8codepointBytes(unsigned char c) {
        if (c < 128) {
            return 1;
        } else if (c < 224) {
            return 2;
        } else if (c < 240) {
            return 3;
        } else {
            return 4;
        }
    }
    static unsigned long UTF8toUTF32 (const char* utf8, unsigned char *nBytes) {
        unsigned char arr[4];
        memset(arr, 0, 4);
        *nBytes = UTF8codepointBytes(*utf8);
        memcpy(arr + 4-*nBytes, utf8, *nBytes);
        arr[4-*nBytes] &= 255 >> *nBytes;
        return ((arr[0] & 63) << 18  |  (arr[1] & 63) << 12  |
                (arr[2] & 63) << 6  |  (arr[3] & 127));
    }

    double GetStrWidth(const char *str) const {
        double w = 0;
        unsigned int length = strlen(str);
        unsigned char len;
        unsigned long prevCh, nextCh;
        nextCh = UTF8toUTF32(str, &len);
        unsigned int i;
        for (i = len;  i < length;  i += len) {
            prevCh = nextCh;
            nextCh = UTF8toUTF32(str+i, &len);
            w += GetAdvance(prevCh, nextCh);
        }
        //last character width
        double asc, desc, width;
        GetMetrics(nextCh, asc, desc, width);
        w += width;
        return w;
    }
    
    /******** *nix specific ********/
#ifndef __APPLE__
#ifndef WIN32
    
#ifdef HAVE_ZLIB
    static std::map<std::string, std::vector<std::string> > afmPathDB;
    static std::string packagePath;

    struct SCharMetric {
        int code;
        int w;
        std::string name;
        int llx, lly, urx, ury; //bounding box extents
        double width;
        double ascent;
        double descent;
        SCharMetric(void) {
            width = ascent = descent = 0;
        }
    };
    typedef std::map<unsigned int,SCharMetric> TMetrics;
    TMetrics m_AFMCharMetrics;
    SCharMetric m_AFMFontBBox;

    struct SCharPair {
        unsigned int prev, next;
        SCharPair(unsigned int p, unsigned int n) : prev(p), next(n) {}
        friend bool operator< (const SCharPair &p1, const SCharPair &p2) {
            return memcmp(&p1,&p2, sizeof(SCharPair)) < 0;
        }
    };
    typedef std::map<SCharPair, double> TKerningTable;
    TKerningTable m_AFMKerningTable;
#endif
#ifdef HAVE_XFT
    static Display *s_XDisplay; //global connection to X server
    XftFont *m_FontInfo;
#endif

    SSysFontInfo(const SFontSpec& spec) : m_Spec(spec) {
#ifdef HAVE_ZLIB
        if (afmPathDB.size() == 0) {
            afmPathDB["Courier"].push_back("Courier-ucs.afm");
            afmPathDB["Courier"].push_back("Courier-Bold-ucs.afm");
            afmPathDB["Courier"].push_back("Courier-Oblique-ucs.afm");
            afmPathDB["Courier"].push_back("Courier-BoldOblique-ucs.afm");
            afmPathDB["Helvetica"].push_back("Helvetica-ucs.afm");
            afmPathDB["Helvetica"].push_back("Helvetica-Bold-ucs.afm");
            afmPathDB["Helvetica"].push_back("Helvetica-Oblique-ucs.afm");
            afmPathDB["Helvetica"].push_back("Helvetica-BoldOblique-ucs.afm");
            afmPathDB["sans"] = afmPathDB["Helvetica"];
            afmPathDB["Times"].push_back("Times-Roman-ucs.afm");
            afmPathDB["Times"].push_back("Times-Bold-ucs.afm");
            afmPathDB["Times"].push_back("Times-Italic-ucs.afm");
            afmPathDB["Times"].push_back("Times-BoldItalic-ucs.afm");
            afmPathDB["serif"] = afmPathDB["times"];
            afmPathDB["ZapfDingbats"].push_back("ZapfDingbats-ucs.afm");
            afmPathDB["ZapfDingbats"].push_back("ZapfDingbats-ucs.afm");
            afmPathDB["ZapfDingbats"].push_back("ZapfDingbats-ucs.afm");
            afmPathDB["ZapfDingbats"].push_back("ZapfDingbats-ucs.afm");
            afmPathDB["Symbol"].push_back("Symbol-ucs.afm");
            afmPathDB["Symbol"].push_back("Symbol-ucs.afm");
            afmPathDB["Symbol"].push_back("Symbol-ucs.afm");
            afmPathDB["Symbol"].push_back("Symbol-ucs.afm");

            //find full path to package using R "findPackage" function
            SEXP findPackage, call;
            PROTECT(findPackage = Rf_findFun(Rf_install("find.package"),
                                             R_GlobalEnv));
            PROTECT(call = Rf_lang2(findPackage, Rf_ScalarString
                                    (Rf_mkChar("devEMF"))));
            SEXP res = Rf_eval(call, R_GlobalEnv);
            UNPROTECT(2);
            if (!Rf_isVector(res)  ||  !Rf_isString(res)  ||
                Rf_length(res) != 1) {
                Rf_error("find.package failed to find devEMF install location"
                         " (or uniquely identify location)");
            }
            packagePath = CHAR(STRING_ELT(res, 0));
        }
#endif
#ifdef HAVE_XFT
        m_FontInfo = NULL;
        if (!s_XDisplay) {
            s_XDisplay = XOpenDisplay(NULL);
            if (!s_XDisplay) {
#ifndef HAVE_ZLIB
                Rf_error("Can't open connection to X server to read font "
                         "metric information (and devEMF was not compiled "
                         "with zlib support to allow pulling metrics from "
                         "file).");
#endif
            }
        }
#endif

#ifdef HAVE_XFT
        if (s_XDisplay) {
            m_FontInfo = XftFontOpen
                (s_XDisplay, XDefaultScreen(s_XDisplay),
                 XFT_FAMILY, XftTypeString, m_Spec.m_Family.c_str(),
                 XFT_PIXEL_SIZE, XftTypeInteger, m_Spec.m_Size,
                 XFT_SLANT, XftTypeInteger, (m_Spec.m_Face == 3  ||
                                             m_Spec.m_Face == 4 ?
                                             XFT_SLANT_ITALIC :
                                             XFT_SLANT_ROMAN),
                 XFT_WEIGHT, XftTypeInteger, (m_Spec.m_Face == 2  ||
                                              m_Spec.m_Face == 4 ?
                                              XFT_WEIGHT_BOLD :
                                              XFT_WEIGHT_MEDIUM),
                 NULL);
            if (m_FontInfo) {
                FT_Face face = XftLockFace(m_FontInfo);
                if (m_Spec.m_Family != face->family_name) {
                    Rf_warning("devEMF: your system substituted font family '%s' when you requested '%s'",
                               face->family_name, m_Spec.m_Family.c_str());
                }
                XftUnlockFace(m_FontInfo);
                return;
            } //otherwise proceed to AFM files
        }
#endif
#ifdef HAVE_ZLIB
        if (afmPathDB.find(m_Spec.m_Family) == afmPathDB.end()  ||
            afmPathDB[m_Spec.m_Family].size() < m_Spec.m_Face) {
#ifdef HAVE_XFT
            Rf_warning("devEMF: font metric information not found for family '%s'; "
                       "using 'Helvetica' instead", m_Spec.m_Family.c_str());
#else
            Rf_warning("devEMF: font metric information not available for family '%s'; "
                       "using 'Helvetica' instead (consider installing Xft through a system level-package called 'libxft-dev' or similar and then reinstall the devEMF package).", m_Spec.m_Family.c_str());
#endif

            //last-ditch substitute with "Helvetica"
            LoadAFM((packagePath+"/afm/" +
                     afmPathDB["Helvetica"][m_Spec.m_Face-1] + ".gz").
                    c_str(), m_Spec.m_Size, true);
        } else {
            LoadAFM((packagePath+"/afm/" +
                     afmPathDB[m_Spec.m_Family][m_Spec.m_Face-1] + ".gz").
                    c_str(), m_Spec.m_Size, true);
        }
        //populate extra characters
        if (m_Spec.m_Family != "Symbol") {
            LoadAFM((packagePath+"/afm/" +
                     afmPathDB["Symbol"][m_Spec.m_Face-1] + ".gz").
                    c_str(), m_Spec.m_Size, false);
        }
        if (m_Spec.m_Family != "ZapfDingbats") {
            LoadAFM((packagePath+"/afm/" +
                     afmPathDB["ZapfDingbats"][m_Spec.m_Face-1] + ".gz").
                    c_str(), m_Spec.m_Size, false);
        }
#endif
    }
#ifdef HAVE_XFT
    ~SSysFontInfo() {
        if (m_FontInfo) {
            XftFontClose(s_XDisplay, m_FontInfo);
        }
    }
#endif

#ifdef HAVE_ZLIB
    void LoadAFM(const std::string &filename, int size, bool loadFontBBox) {
        typedef std::map<std::string, unsigned int> TName2Code;
        TName2Code name2code;
        const unsigned int buffsize = 512;
        char buff[buffsize];
        gzFile afm = gzopen(filename.c_str(), "rb");
        while (gzgets(afm, buff, buffsize)) {
            std::stringstream iss(buff);
            std::string key;
            iss >> key;
            if (key == "FontBBox"  &&  loadFontBBox) {
                iss >> m_AFMFontBBox.llx >> m_AFMFontBBox.lly
                    >> m_AFMFontBBox.urx >> m_AFMFontBBox.ury;
                m_AFMFontBBox.ascent = m_AFMFontBBox.ury * 0.001 * size;
                m_AFMFontBBox.descent = m_AFMFontBBox.lly * 0.001 * size;
                m_AFMFontBBox.width = (m_AFMFontBBox.urx-m_AFMFontBBox.llx)
                    * 0.001 * size;
            } else if (key == "C") {
                SCharMetric cMetric;
                iss >> std::hex >> cMetric.code >> std::dec >> key;
                while (iss.good()) {
                    if (key == "WX") {
                        iss >> cMetric.w;
                        cMetric.width = cMetric.w * 0.001 * size;
                    } else if (key == "N") {
                        iss >> cMetric.name;
                    } else if (key == "B") {
                        iss >> cMetric.llx >> cMetric.lly
                            >> cMetric.urx >> cMetric.ury;
                        cMetric.ascent = cMetric.ury * 0.001 * size;
                        cMetric.descent = cMetric.lly * 0.001 * size;
                    }
                    iss >> key;
                }
                if (m_AFMCharMetrics.find(cMetric.code) ==
                    m_AFMCharMetrics.end()) {
                    m_AFMCharMetrics[cMetric.code] = cMetric;
                    name2code[cMetric.name] = cMetric.code;
                }
            } else if (key == "KPX") {
                std::string name1, name2;
                iss >> name1 >> name2;
                TName2Code::const_iterator ch1 = name2code.find(name1);
                TName2Code::const_iterator ch2 = name2code.find(name2);
                if (ch1 != name2code.end()  &&
                    ch2 != name2code.end()) {
                    int kern;
                    iss >> kern;
                    m_AFMKerningTable[SCharPair(ch1->second, ch2->second)] = kern * 0.001 * size;
                }
            }
        }
        gzclose(afm);
    }
#endif

    bool HasChar(unsigned int c) const {
#ifdef HAVE_XFT
        if (m_FontInfo) {
            return XftCharExists(s_XDisplay, m_FontInfo, c);
        } else {
            return false;
        }
#endif
#ifdef HAVE_ZLIB
        return m_AFMCharMetrics.find(c) != m_AFMCharMetrics.end();
#endif
    }
    
#ifdef HAVE_XFT
    struct SPathOutlineFuncs : public FT_Outline_Funcs {
        SPathOutlineFuncs(void) {
            move_to = (FT_Outline_MoveToFunc) MoveTo;
            line_to = (FT_Outline_LineToFunc) LineTo;
            conic_to = (FT_Outline_ConicToFunc) ConicTo;
            cubic_to = (FT_Outline_CubicToFunc) CubicTo;
            shift = 0;
            delta = 0;
        }
        static int MoveTo(const FT_Vector* to, EMFPLUS::SPath *path) {
            //note coordinates are in 26.6 fixed-point format
            path->StartNewPoly((double)to->x/64, (double)to->y/64);
            //Rprintf("start: %d, %d\n", to->x, to->y);
            return 0;
        }
        static int LineTo(const FT_Vector* to, EMFPLUS::SPath *path) {
            path->AddLineTo((double)to->x/64, (double)to->y/64);
            //Rprintf("line: %d, %d\n", to->x, to->y);
            return 0;
        }
        static int ConicTo(const FT_Vector* control, const FT_Vector* to,
                           EMFPLUS::SPath *path) {
            path->AddQuadBezierTo((double)control->x/64,
                                  (double)control->y/64,
                                  (double)to->x/64,
                                  (double)to->y/64);
            //Rprintf("quad: (%d,%d), %d, %d\n", control->x, control->y, to->x, to->y);
            return 0;
        }
        static int CubicTo(const FT_Vector* control1, const FT_Vector* control2,
                           const FT_Vector* to, EMFPLUS::SPath *path) {
            path->AddCubicBezierTo((double)control1->x/64,
                                   (double)control1->y/64,
                                   (double)control2->x/64,
                                   (double)control2->y/64,
                                   (double)to->x/64, (double)to->y/64);
            //Rprintf("cubic: (%d,%d), (%d,%d) %d, %d\n", control1->x, control1->y, control2->x, control2->y, to->x, to->y);
            return 0;
        }
    };
#endif

    void AppendGlyphPath(unsigned int c, EMFPLUS::SPath &path) const {
#ifdef HAVE_XFT
        if (!m_FontInfo) {
            Rf_error("devEMF: font (%s) not found by Xft so can't embed fonts!",
                     m_Spec.m_Family.c_str());
        }
        FT_Face face = XftLockFace(m_FontInfo);
        FT_Matrix transform; //note flipping around y for emf's coord system
        transform.xx = 65536; transform.xy = 0;
        transform.yx = 0; transform.yy = -65536;
        FT_Set_Transform(face, &transform, NULL);
        FT_Set_Pixel_Sizes(face, m_Spec.m_Size, 0);
        int err = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
        if (err != 0) {
            Rf_warning("devEMF: could not find font outline for embedding '%c'",c);
        }
        if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
            SPathOutlineFuncs myFuncs;
            FT_Outline_Decompose(&face->glyph->outline,
                                 &myFuncs, &path);
        }
        XftUnlockFace(m_FontInfo);
        //Rprintf("totalpts: %d\n", path.m_TotalPts);
        return;
#endif
        Rf_error("devEMF: Font to path conversion requires devEMF to be compiled with Xft and FreeType (perhaps you need to first install system level-packages called 'libxft-dev' and 'libfreetype-dev' and then reinstall the devEMF package).");
    }

    // actual advance (accounting for kerning!)
    double GetAdvance(unsigned long prevC, unsigned long nextC) const {
#ifdef HAVE_XFT
        if (m_FontInfo) {
            FT_Face face = XftLockFace(m_FontInfo);
            //Rprintf("\nsize scaling:  %u, %i,%i, %f, %f\n", face->units_per_EM, face->size->metrics.x_ppem, face->size->metrics.y_ppem, face->size->metrics.x_scale/(double) 65536, face->size->metrics.y_scale / (double) 65536);
            unsigned int prevI = FT_Get_Char_Index(face, prevC);
            unsigned int nextI = FT_Get_Char_Index(face, nextC);
            FT_Vector kerning;
            FT_Get_Kerning(face, prevI, nextI, FT_KERNING_DEFAULT, &kerning);
            XftUnlockFace(m_FontInfo);
            FT_Load_Glyph(face, prevI, FT_LOAD_NO_BITMAP);
            //Rprintf("kkkerning %u %u %f %f %f\n", prevC, nextC, face->glyph->advance.x/(double)64, kerning.x/(double)64, face->glyph->linearHoriAdvance/(double)65536);
            return (face->glyph->advance.x + (face->glyph->lsb_delta - face->glyph->rsb_delta) + kerning.x)/(double)64;
        }
#endif
        double wid = 0;
#ifdef HAVE_ZLIB
        double asc, desc;
        GetMetrics(prevC, asc, desc, wid);
        TKerningTable::const_iterator kernI = m_AFMKerningTable.find(SCharPair(prevC, nextC));
        if (kernI != m_AFMKerningTable.end()) {
             wid += kernI->second;
             //Rprintf("kkkerning %u %u %f\n", prevC, nextC, kernI->second);
        }
#endif
        return wid;
    }
    
    void GetMetrics(unsigned int c, 
                    double &ascent, double &descent, double &width) const {
#ifdef HAVE_XFT
        if (m_FontInfo) {
            XGlyphInfo extents;
            XftTextExtents32(s_XDisplay, m_FontInfo, &c, 1, &extents);
            // See below URL for interpreting XFT extents
            // http://ns.keithp.com/pipermail/fontconfig/2003-June/000492.html
            ascent = extents.y;
            descent = extents.height-extents.y;
            width = extents.xOff;
            return;
        }
#endif
#ifdef HAVE_ZLIB
        TMetrics::const_iterator m= m_AFMCharMetrics.find(c);
        if (m == m_AFMCharMetrics.end()) {
            ascent = 0;
            descent = 0;
            width = 0;
        } else {
            ascent = m->second.ascent;
            descent = m->second.descent;
            width = m->second.width;
        }
#endif
    }
    void GetFontBBox(double &ascent, double &descent, double &width) {
        ascent = descent = width = 0;
#ifdef HAVE_XFT
        if (m_FontInfo) {
            ascent = m_FontInfo->ascent;
            descent = m_FontInfo->descent;
            width = m_FontInfo->max_advance_width;
            return;
        }
#endif
#ifdef HAVE_ZLIB
        ascent = m_AFMFontBBox.ascent;
        descent = m_AFMFontBBox.descent;
        width = m_AFMFontBBox.width;
#endif        
    }
};
#ifdef HAVE_XFT
Display* SSysFontInfo::s_XDisplay = NULL; //global connection to X server
#endif
#ifdef HAVE_ZLIB
std::map<std::string, std::vector<std::string> > SSysFontInfo::afmPathDB;
std::string SSysFontInfo::packagePath;
#endif

#endif /* end not windows */
#endif /* end not mac */

/**** windows specific ****/
#ifdef WIN32
    HDC m_DC;
    HFONT m_FontHandle;
    struct SCharPair {
        unsigned int prev, next;
        SCharPair(unsigned int p, unsigned int n) : prev(p), next(n) {}
        friend bool operator< (const SCharPair &p1, const SCharPair &p2) {
            return memcmp(&p1,&p2, sizeof(SCharPair)) < 0;
        }
    };
    typedef std::map<SCharPair, int> TKerningTable;
    TKerningTable m_KerningTable;

    SSysFontInfo(const SFontSpec& spec) : m_Spec(spec) {
        m_DC = GetDC(0);
        LOGFONT lf;
        lf.lfHeight = -m_Spec.m_Size;//(-) matches against *character* height
        lf.lfWidth = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = (m_Spec.m_Face == 2  ||  m_Spec.m_Face == 4) ?
            FW_BOLD : FW_NORMAL;
        lf.lfItalic = (m_Spec.m_Face == 3  ||  m_Spec.m_Face == 4) ? 0x01 : 0x00;
        lf.lfUnderline = 0x00;
        lf.lfStrikeOut = 0x00;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_STROKE_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfPitchAndFamily = FF_DONTCARE + DEFAULT_PITCH;
        MultiByteToWideChar(CP_UTF8, 0, m_Spec.m_Family.c_str(), -1,
                            lf.lfFaceName, LF_FACESIZE);
        m_FontHandle = CreateFontIndirect(&lf);
        SelectObject(m_DC, m_FontHandle);

        unsigned int nPairs = GetKerningPairsW(m_DC, 0, NULL);
        if (nPairs > 0) {
            KERNINGPAIR *kernPairs = new KERNINGPAIR[nPairs];
            GetKerningPairsW(m_DC, nPairs, kernPairs);
            for (unsigned int i = 0;  i < nPairs;  ++i) {
                m_KerningTable[SCharPair(kernPairs[i].wFirst,
                                         kernPairs[i].wSecond)] =
                    kernPairs[i].iKernAmount;
            }
            delete[] kernPairs;
        }
    }
    ~SSysFontInfo(void) {
        DeleteObject(m_FontHandle);
        ReleaseDC(0, m_DC);
    }
    bool HasChar(unsigned int c) const {
        GLYPHMETRICS metrics;
        const MAT2 matrix = {{0,1}, {0,0}, {0,0}, {0,1}};
        return (GetGlyphOutlineW(m_DC, c, GGO_METRICS, &metrics,
                                 0, NULL, &matrix) != GDI_ERROR);
    }

    //fixed to double
    static double F2D(const FIXED& f) {
        return (double)f.value + ((double)f.fract)/65536;
    }

    void AppendGlyphPath(unsigned int c, EMFPLUS::SPath &path) const {
        GLYPHMETRICS metrics;
        MAT2 transform;
        memset(&transform, 0, sizeof(MAT2));
        transform.eM11.value = 1;
        transform.eM22.value = -1;
        unsigned int buffSize = GetGlyphOutlineW(m_DC, c, GGO_NATIVE, &metrics,
                                                 0, NULL, &transform);

        if (buffSize == GDI_ERROR) {
            Rf_warning("devEMF: glyph outline retrieval failed; "
                       "try with emPlusFontToPath=FALSE");
            return;
        }
        char* buff = new char[buffSize];
        GetGlyphOutlineW(m_DC, c, GGO_NATIVE, &metrics, buffSize, buff,
                         &transform);
        
        char* offset = buff;
        while (offset + sizeof(TTPOLYGONHEADER) <= buff + buffSize) {
            TTPOLYGONHEADER* header = (TTPOLYGONHEADER*) offset;
            path.StartNewPoly(F2D(header->pfxStart.x),
                              F2D(header->pfxStart.y));
            TTPOLYCURVE* curve = (TTPOLYCURVE*)(offset+sizeof(TTPOLYGONHEADER));
            while ((char*)curve < offset + header->cb) {
                for (unsigned int i = 0;  i < curve->cpfx;  ++i) {
                    double x = F2D(curve->apfx[i].x);
                    double y = F2D(curve->apfx[i].y);
                    if (curve->wType == TT_PRIM_LINE) {
                        path.AddLineTo(x, y);
                    } else if (curve->wType == TT_PRIM_QSPLINE) {
                        if (i+2 == curve->cpfx) {
                            ++i;
                            path.AddQuadBezierTo(x, y,
                                                 F2D(curve->apfx[i].x),
                                                 F2D(curve->apfx[i].y));
                        } else if (i+1 < curve->cpfx) {
                            path.AddQuadBezierTo(x, y,
                                                 (F2D(curve->apfx[i+1].x)+x)/2,
                                                 (F2D(curve->apfx[i+1].y)+y)/2);
                        }
                    } else if (curve->wType == TT_PRIM_CSPLINE) {
                        Rf_warning("devEMF: unimplemented TT_PRIM_CSPLINE; "
                                   "contact devEMF author");
                    }
                } // end of each point
                curve = (TTPOLYCURVE*)((char*)curve + sizeof(TTPOLYCURVE) + sizeof(curve->apfx)*(curve->cpfx-1));
            } // end of each curve
            path.CloseCurrPoly();
            offset += header->cb;
        } // end of each contour
        delete[] buff;
    }
    double GetAdvance(unsigned long prevC, unsigned long nextC) const {
        double ascent, descent, width;
        GetMetrics(prevC, ascent, descent, width);
        TKerningTable::const_iterator i =
            m_KerningTable.find(SCharPair(prevC, nextC));
        if (i != m_KerningTable.end()) {
            width += i->second;
        }
        return width;
    }

    void GetMetrics(unsigned int c, 
                    double &ascent, double &descent, double &width) const {
        GLYPHMETRICS metrics;
        const MAT2 matrix = {{0,1}, {0,0}, {0,0}, {0,1}};
        if (GetGlyphOutlineW(m_DC, c, GGO_METRICS, &metrics, 0, NULL,&matrix) ==
            GDI_ERROR) {
            ascent = 0;
            descent = 0;
            width = 0;
        } else {
            ascent = metrics.gmptGlyphOrigin.y;
            descent = ((int)metrics.gmBlackBoxY <= metrics.gmptGlyphOrigin.y) ?
                0 : metrics.gmBlackBoxY - metrics.gmptGlyphOrigin.y;
            width = metrics.gmCellIncX;
        }
    }
    void GetFontBBox(double &ascent, double &descent, double &width) {
        TEXTMETRIC metrics;
        GetTextMetrics(m_DC, &metrics);
        ascent = metrics.tmAscent;
        descent = metrics.tmDescent;
        width = metrics.tmAveCharWidth;
    }
};

#endif /* end WIN32 */

/**** apple specific ****/

#ifdef __APPLE__
    CTFontRef m_FontInfo;

    SSysFontInfo(const SFontSpec& spec) : m_Spec(spec) {
        CFMutableDictionaryRef attr =
            CFDictionaryCreateMutable(NULL, 1, &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
        CFStringRef vFam =
            CFStringCreateWithCString(NULL, spec.m_Family.c_str(),
                                      kCFStringEncodingUTF8);
        CFDictionaryAddValue(attr, kCTFontFamilyNameAttribute, vFam);
        float fSize = m_Spec.m_Size;
        CFNumberRef vSize = CFNumberCreate(NULL, kCFNumberFloatType, &fSize);
        CFDictionaryAddValue(attr, kCTFontSizeAttribute, vSize);
        CFMutableDictionaryRef vTrait =
            CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
        float fWeight = (m_Spec.m_Face == 2  ||  m_Spec.m_Face == 4 ? 1 : 0);
        CFNumberRef vWeight =CFNumberCreate(NULL, kCFNumberFloatType, &fWeight);
        float fSlant = (m_Spec.m_Face == 3  ||  m_Spec.m_Face == 4 ? 1 : 0);
        CFNumberRef vSlant = CFNumberCreate(NULL, kCFNumberFloatType, &fSlant);
        CFDictionaryAddValue(vTrait, kCTFontWeightTrait, vWeight);
        CFDictionaryAddValue(vTrait, kCTFontSlantTrait, vSlant);
        CFDictionaryAddValue(attr, kCTFontTraitsAttribute, vTrait);
        CTFontDescriptorRef descriptor =
            CTFontDescriptorCreateWithAttributes(attr);
        m_FontInfo = CTFontCreateWithFontDescriptor
            (descriptor, m_Spec.m_Size, NULL);
        CFRelease(descriptor);
        CFRelease(attr);
        CFRelease(vSlant);
        CFRelease(vWeight);
        CFRelease(vTrait);
        CFRelease(vSize);
        CFRelease(vFam);
    }
    ~SSysFontInfo() {CFRelease(m_FontInfo);}
    bool HasChar(unsigned int c) const {
        CGGlyph glyph;
        UniChar ch = c;
        return CTFontGetGlyphsForCharacters (m_FontInfo, &ch, &glyph, 1);
    }
    void AppendGlyphPath(unsigned int c, EMFPLUS::SPath &path) const {
        Rf_error("devEMF: Font to path conversion not implemented for Apple; contact author.");
    }
    double GetAdvance(unsigned long prevC, unsigned long nextC) const {
        CGGlyph glyph[2];
        UniChar ch[2];
        ch[0] = prevC;
        ch[1] = nextC;
        CTFontGetGlyphsForCharacters (m_FontInfo, ch, glyph, 2);
        CGSize adv[2];
        CTFontGetAdvancesForGlyphs(m_FontInfo,
                                   kCTFontOrientationDefault,
                                   glyph, adv, 2);
        return adv[0].width;
    }
    void GetMetrics(unsigned int c, 
                    double &ascent, double &descent, double &width) const {
        CGGlyph glyph;
        UniChar ch = c;
        CTFontGetGlyphsForCharacters (m_FontInfo, &ch, &glyph, 1);
        CGRect extents;
        extents = CTFontGetBoundingRectsForGlyphs(m_FontInfo,
                                                  kCTFontOrientationDefault,
                                                  &glyph, NULL, 1);
        ascent = extents.origin.y+extents.size.height;
        descent = (extents.origin.y > 0) ? 0 : -extents.origin.y;
        width = CTFontGetAdvancesForGlyphs(m_FontInfo,
                                           kCTFontOrientationDefault,
                                           &glyph, NULL, 1);
    }
    void GetFontBBox(double &ascent, double &descent, double &width) {
        ascent = CTFontGetAscent(m_FontInfo);
        descent = CTFontGetDescent(m_FontInfo);
        width = 0;//todo
    }
};

#endif /* end __APPLE__ */
