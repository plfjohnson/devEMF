/* $Id: emf.h 365 2022-11-04 20:22:16Z pjohnson $
    --------------------------------------------------------------------------
    Add-on package to R to produce EMF graphics output (for import as
    a high-quality vector graphic into Microsoft Office or OpenOffice).


    Copyright (C) 2011 Philip Johnson

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
    --------------------------------------------------------------------------
*/

#ifndef EMF__H
#define EMF__H

#include <stdexcept>
#include <string>
#include <vector>
#include <math.h>

namespace EMF {
    struct ofstream : std::ofstream {
        bool inEMFplus;
        unsigned int nRecords;
        std::streampos emfPlusStartPos;
        ofstream(void) : std::ofstream() { inEMFplus = false; nRecords = 0;}
    };
}

//forward declaration from emf+.h
namespace EMFPLUS {
    void GetDC(EMF::ofstream &o);
}

// structs for EMF
namespace EMF {
    enum ERecordType {
        eEMR_HEADER = 1,
        eEMR_POLYGON = 3,
        eEMR_POLYLINE = 4,
        eEMR_SETBRUSHORGEX = 13,
        eEMR_EOF = 14,
        eEMR_SETMAPMODE = 17,
        eEMR_SETBKMODE = 18,
        eEMR_SETPOLYFILLMODE = 19,
        eEMR_SETSTRETCHBLTMODE = 21,
        eEMR_SETTEXTALIGN = 22,
        eEMR_SETTEXTCOLOR = 24,
        eEMR_SAVEDC = 33,
        eEMR_RESTOREDC = 34,
        eEMR_SETWORLDTRANSFORM = 35,
        eEMR_MODIFYWORLDTRANSFORM = 36,
        eEMR_SELECTOBJECT = 37,
        eEMR_CREATEPEN = 38,
        eEMR_CREATEBRUSHINDIRECT = 39,
        eEMR_ELLIPSE = 42,
        eEMR_RECTANGLE = 43,
        eEMR_SETMITERLIMIT = 58,
        eEMR_COMMENT = 70,
        eEMR_EXTSELECTCLIPRGN = 0x4B,
        eEMR_INTERSECTCLIPRECT = 0x1E,
        eEMR_BITBLT = 76,
        eEMR_STRETCHBLT = 77,
        eEMR_STRETCHDIBITS = 81,
        eEMR_EXTCREATEFONTINDIRECTW = 82,
        eEMR_EXTTEXTOUTW = 84,
        eEMR_EXTCREATEPEN = 95,
        eEMR_last = 255 //placeholder for max value
    };

    enum EPenStyle {
        ePS_ENDCAP_ROUND  = 0x00000000,
        ePS_JOIN_ROUND    = 0x00000000,
        ePS_SOLID         = 0x00000000,
        ePS_DASH          = 0x00000001,
        ePS_DOT           = 0x00000002,
        ePS_DASHDOT       = 0x00000003,
        ePS_DASHDOTDOT    = 0x00000004,
        ePS_NULL          = 0x00000005,
        ePS_INSIDEFRAME   = 0x00000006,
        ePS_USERSTYLE     = 0x00000007,
        ePS_ALTERNATE     = 0x00000008,
        ePS_ENDCAP_SQUARE = 0x00000100,
        ePS_ENDCAP_FLAT   = 0x00000200,
        ePS_JOIN_BEVEL    = 0x00001000,
        ePS_JOIN_MITER    = 0x00002000,
        ePS_GEOMETRIC     = 0x00010000
    };

    enum EBrushStyle {
        eBS_SOLID = 0,
        eBS_NULL  = 1
    };
    
    enum EFontCharset {
        eANSI_CHARSET = 0,
        eDEFAULT_CHARSET = 1,
        eSYMBOL_CHARSET  = 2
    };

    enum EFontOutPrecision {
        eOUT_DEFAULT_PRECIS   = 0,
        eOUT_STRING_PRECIS    = 1,
        eOUT_CHARACTER_PRECIS = 2,
        eOUT_STROKE_PRECIS    = 3,
        eOUT_TT_PRECIS        = 4
    };

    enum EFontClipPrecision {
        eCLIP_DEFAULT_PRECIS    = 0x00,
        eCLIP_CHARACTER_PRECIS  = 0x01,
        eCLIP_STROKE_PRECIS     = 0x02
    };

    enum EFontQuality {
        eDEFAULT_QUALITY       = 0,
        eDRAFT_QUALITY         = 1,
        ePROOF_QUALITY         = 2,
        eNONANTIALIASED_QUALITY= 3,
        eANTIALIASED_QUALITY   = 4,
        eCLEARTYPE_QUALITY     = 5
    };

    enum EFontPitchFamily {
        eDEFAULT_PITCH      = 0x00,
        eFIXED_PITCH        = 0x01,
        eVARIABLE_PITCH     = 0x02,

        eFF_DONTCARE        = 0x00,
        eFF_ROMAN           = 0x10,
        eFF_SWISS           = 0x20,
        eFF_MODERN          = 0x30,
        eFF_SCRIPT          = 0x40,
        eFF_DECORATIVE      = 0x50
    };

    enum ETextAlign {
        eTA_LEFT            = 0x00,
        eTA_TOP             = 0x00,
        eTA_RIGHT           = 0x02,
        eTA_CENTER          = 0x06,
        eTA_BOTTOM          = 0x08,
        eTA_BASELINE        = 0x18
    };

    enum ETextOptions {
        eETO_NO_RECT        = 0x100
    };

    enum EBkMode {
        eTRANSPARENT      = 1,
        eOPAQUE           = 2
    };

    enum EPolyFillMode {
        ePF_ALTERNATE      = 1,
        ePF_WINDING        = 2
    };

    enum EMapMode {
        eMM_TEXT         = 1,
        eMM_LOMETRIC	 = 2,
        eMM_HIMETRIC	 = 3,
        eMM_LOENGLISH	 = 4,
        eMM_HIENGLISH	 = 5,
        eMM_TWIPS	 = 6,
        eMM_ISOTROPIC	 = 7,
        eMM_ANISOTROPIC	 = 8
    };

    enum EGraphicsMode {
        eGM_COMPATIBLE    = 1,
        eGM_ADVANCED      = 2
    };

    enum EModifyWorldTransformMode {
        eMWT_IDENTITY        = 1,
        eMWT_LEFTMULTIPLY    = 2,
        eMWT_RIGHTMULTIPLY   = 3,
        eMWT_SET             = 4
    };

    // ------------------------------------------------------------------------
    // Generic objects for specified bytes of little-endian data storage

    template<typename TType, size_t nBytes> class CLEType {
    public:
        char m_Val[nBytes];

        CLEType(void) {}
        CLEType(TType v) {
            if (sizeof(v) < nBytes) {
                throw std::logic_error("sizeof(v) < nBytes");
            }
            *this = v;
        }
        CLEType& operator= (TType v) {
            //store as little-endian
            unsigned char *ch = reinterpret_cast<unsigned char*>(&v);
            for (unsigned int i = 0;  i < nBytes;  ++i) {
#ifdef WORDS_BIGENDIAN
                m_Val[i] = ch[sizeof(TType) - i - 1];
#else
                m_Val[i] = ch[i];
#endif
            }
            //make sure we get signed bit if sizeof(TType) > nBytes
            if (sizeof(TType) > nBytes) {
#ifdef WORDS_BIGENDIAN
                m_Val[nBytes-1] |= ch[0] & 0x80;
#else
                m_Val[nBytes-1] |= ch[sizeof(TType) - 1] & 0x80;
#endif
            }
            return *this;
        }

        bool operator< (const CLEType &other) const {
            return memcmp(m_Val, other.m_Val, nBytes) < 0;
        }
        
        friend std::string& operator<< (std::string &o, const CLEType &d) {
            o.append(d.m_Val, nBytes);
            return o;
        }
    };
    typedef CLEType<unsigned int,   4> TUInt4;
    typedef CLEType<unsigned short, 2> TUInt2;
    typedef CLEType<unsigned char,  1> TUInt1;
    typedef CLEType<int, 4>   TInt4;
    typedef CLEType<float, 4> TFloat4;

    // ------------------------------------------------------------------------
    // EMF Objects used repeatedly

    struct SPoint {
        int x, y;
        void Set(int xx, int yy) { x = xx; y = yy; }
        friend std::string& operator<< (std::string &o, const SPoint &d) {
            return o << TInt4(d.x) << TInt4(d.y);
        }
    };

    struct SSize {
        unsigned int cx, cy;
        void Set(unsigned int xx, unsigned int yy) { cx = xx; cy = yy; }
        friend std::string& operator<< (std::string &o, const SSize &d) {
            return o << TUInt4(d.cx) << TUInt4(d.cy);
        }
    };
    
    struct SRect {
        int left, top, right, bottom;
        void Set(int l, int t, int r, int b) {
            left = l; top = std::min(t,b); right = r; bottom = std::max(t,b);
        }
        friend std::string& operator<< (std::string &o, const SRect &d) {
            return o << TInt4(d.left) << TInt4(d.top)
                     << TInt4(d.right) << TInt4(d.bottom);
        }
    };

    struct SColorRef {
        TUInt1 red, green, blue, reserved;
        void Set(unsigned char r, unsigned char g, unsigned char b) {
            red = r;  green = g;  blue = b; reserved=0x00;
        }
        friend std::string& operator<< (std::string &o, const SColorRef &d) {
            //last byte is "reserved"
            return o << d.red << d.green << d.blue << d.reserved;
        }
    };

    struct SXForm {
        TFloat4 m[2][2];
        TFloat4 d[2];
        void Set(double m11, double m12, double m21, double m22,
                 double dx, double dy) {
            m[0][0]=m11; m[0][1]=m12; m[1][0]=m21; m[1][1]=m22;
            d[0] = dx; d[1] = dy;
        }
        friend std::string& operator<< (std::string &o, const SXForm &x) {
            return o << x.m[0][0] << x.m[0][1] << x.m[1][0] << x.m[1][1]
                     << x.d[0] << x.d[1];
        }
    };

    // ------------------------------------------------------------------------
    // EMF Records + EMF objects used in only one record begin here

    struct SRecord {
        ERecordType iType;
        TUInt4 nSize;
        SRecord(ERecordType t) : iType(t), nSize(0) {}

        virtual std::string& Serialize(std::string &o) const {
            return o << TUInt4(iType) << nSize;
        }
        void Write(EMF::ofstream &o) {
            if (o.inEMFplus) {
                EMFPLUS::GetDC(o); // emf+ record to enable reading of emf
                o.inEMFplus = false;
            }
            ++o.nRecords;
            std::string buff; Serialize(buff);
            buff.resize(((buff.size() + 3)/4)*4, '\0'); //add padding
            std::string finalSize; finalSize << TUInt4(buff.size());
            buff.replace(4,4, finalSize);
            o.write(buff.data(), buff.size());
        }
};

    struct SHeader : SRecord {
        SRect bounds;
        SRect frame;
        unsigned int signature;
        unsigned int version;
        unsigned int nBytes;
        unsigned int nRecords;
        unsigned short nHandles;
        unsigned short reserved;
        unsigned int nDescription;
        unsigned int offDescription;
        unsigned int nPalEntries;
        SSize device;
        SSize millimeters;
        unsigned int cbPixelFormat;
        unsigned int offPixelFormat;
        unsigned int bOpenGL;
        SSize micrometers;
        std::string desc;
        SHeader(void) : SRecord(eEMR_HEADER) {}
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << bounds << frame << TUInt4(signature) << TUInt4(version) << TUInt4(nBytes) << TUInt4(nRecords) << TUInt2(nHandles) << TUInt2(reserved) << TUInt4(nDescription) << TUInt4(108) << TUInt4(nPalEntries) << device << millimeters << TUInt4(cbPixelFormat) << TUInt4(offPixelFormat) << TUInt4(bOpenGL) << micrometers;
            o.append(desc);
            return o;
        }
    };

    struct SPlusRecord : SRecord {
        SPlusRecord(void) : SRecord(eEMR_COMMENT) {}
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << nSize;
            o.append("EMF+", 4);
            return o;
        }
    };

    struct SemrText {
        SPoint reference;
        unsigned int  nChars;
        unsigned int  offString;
        unsigned int  options;
        SRect  rect;
        unsigned int  offDx;
        std::string str;
    };
    struct S_EXTTEXTOUTW : SRecord {
        SRect   bounds;
        unsigned int graphicsMode;
        TFloat4  exScale;
        TFloat4  eyScale;
        SemrText emrtext;
        S_EXTTEXTOUTW(void) : SRecord(eEMR_EXTTEXTOUTW) {}
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << bounds << TUInt4(graphicsMode) << exScale << eyScale << emrtext.reference << TUInt4(emrtext.nChars) <<
                TUInt4(19*4) << //offset for string
                TUInt4(emrtext.options) << emrtext.rect <<
                TUInt4(emrtext.offDx);
            o.append(emrtext.str);
            return o;
	}
    };

    struct SObject : SRecord {
        unsigned int m_ObjId;
        SObject(ERecordType t) : SRecord(t) {}
        virtual ~SObject(void) {}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << TUInt4(m_ObjId);
        }
    };

    struct SLogPenEx {
        unsigned int penStyle;
        TUInt4 width;
        TUInt4 brushStyle;
        SColorRef color;
        TUInt4 brushHatch;
        unsigned int numEntries;
    };
    struct SPen : SObject {
        TUInt4 offBmi;
        TUInt4 cbBmi;
        TUInt4 offBits;
        TUInt4 cbBits;
        SLogPenEx elp;
        std::vector<TUInt4> styleEntries;

        SPen(unsigned int col, double lwd, unsigned int lty,
             unsigned int lend, unsigned int ljoin,
             double ps2dev, bool useUserLty) : SObject(eEMR_EXTCREATEPEN) {
            offBmi = cbBmi = offBits = cbBits = 0;
            elp.penStyle = ePS_GEOMETRIC;
            elp.width = lwd*ps2dev;
            elp.brushStyle = eBS_SOLID;
            elp.color.Set(R_RED(col), R_GREEN(col), R_BLUE(col));
            if (R_ALPHA(col) > 0  &&  R_ALPHA(col) < 255) {
                Rf_warning("partial transparency is not supported for EMF "
                           "lines (consider enabling EMF+)");
            }
            elp.brushHatch = 0;
            elp.numEntries = 0;
            if (R_TRANSPARENT(col)) {
                elp.penStyle |= ePS_NULL;
                elp.brushStyle = eBS_NULL;
                return;
            }
            if (!useUserLty) {
                // if not using EMF custom line types, then map
                // between vaguely equivalent default line types
                switch(lty) {
                case LTY_SOLID: elp.penStyle |= ePS_SOLID; break;
                case LTY_DASHED: elp.penStyle |= ePS_DASH; break;
                case LTY_DOTTED: elp.penStyle |= ePS_DOT; break;
                case LTY_DOTDASH: elp.penStyle |= ePS_DASHDOT; break;
                case LTY_LONGDASH: elp.penStyle |= ePS_DASHDOTDOT; break;
                default: elp.penStyle |= ePS_SOLID;
                    Rf_warning("Using lty unsupported by EMF device");
                }
            } else { //custom line style is preferable
                for(int i = 0;  i < 8  &&  lty & 15;  ++i, lty >>= 4) {
                    styleEntries.push_back((lty & 15)*ps2dev);
                }
                if (styleEntries.empty()) {
                    elp.penStyle |= ePS_SOLID;
                } else {
                    elp.penStyle |= ePS_USERSTYLE;
                }
            }

            switch (lend) {
            case GE_ROUND_CAP: elp.penStyle |= ePS_ENDCAP_ROUND; break;
            case GE_BUTT_CAP: elp.penStyle |= ePS_ENDCAP_FLAT; break;
            case GE_SQUARE_CAP: elp.penStyle |= ePS_ENDCAP_SQUARE; break;
            default: break;//actually of range, but R doesn't complain..
            }

            switch (ljoin) {
            case GE_ROUND_JOIN: elp.penStyle |= ePS_JOIN_ROUND; break;
            case GE_MITRE_JOIN: elp.penStyle |= ePS_JOIN_MITER; break;
            case GE_BEVEL_JOIN: elp.penStyle |= ePS_JOIN_BEVEL; break;
            default: break;//actually of range, but R doesn't complain..
            }
        }

        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o) << offBmi << cbBmi << offBits << cbBits << TUInt4(elp.penStyle) << elp.width << elp.brushStyle << elp.color << elp.brushHatch << TUInt4(styleEntries.size());
            for (unsigned int i = 0;  i < styleEntries.size();  ++i) {
                o << styleEntries[i];
            }
            return o;
	}
    };

    struct SLogBrushEx {
        TUInt4    brushStyle;
        SColorRef color;
        TUInt4    brushHatch;
    };
    struct SBrush : SObject {
        SLogBrushEx  lb;

	SBrush(unsigned int col) : SObject(eEMR_CREATEBRUSHINDIRECT) {
            lb.brushStyle = (R_TRANSPARENT(col) ? eBS_NULL : eBS_SOLID);
            lb.color.Set(R_RED(col), R_GREEN(col), R_BLUE(col));
            lb.brushHatch = 0; //unused with BS_SOLID or BS_NULL
            if (R_ALPHA(col) > 0  &&  R_ALPHA(col) < 255) {
                Rf_warning("partial transparency is not supported for EMF "
                           "fills (consider enabling EMF+)");
            }
        }
        std::string& Serialize(std::string &o) const {
            return SObject::Serialize(o) << lb.brushStyle << lb.color << lb.brushHatch;
	}
    };

    enum EFontWeight {
        eFontWeight_normal = 400,
        eFontWeight_bold   = 700
    };
    struct SLogFont {
        TInt4   height;
        TInt4   width;
        TInt4   escapement;
        TInt4   orientation;
        TInt4   weight;
        TUInt1  italic;
        TUInt1  underline;
        TUInt1  strikeOut;
        TUInt1  charSet;
        TUInt1  outPrecision;
        TUInt1  clipPrecision;
        TUInt1  quality;
        TUInt1  pitchAndFamily;
        char    faceName[64]; // <=32 UTF-16 characters
        void SetFace(const std::string &utf16) {
            memset(faceName, 0, 64);//blank out for comparison
            memcpy(faceName, utf16.data(),
                   64 < utf16.length() ? 64 : utf16.length());
        }
    };
    struct SFont : SObject {
        SLogFont lf;
        
        SFont(unsigned char face, int size, const std::string &familyUTF16,
              double rot) :
            SObject(eEMR_EXTCREATEFONTINDIRECTW) {
            lf.height = -size;//(-) matches against *character* height
            lf.width = 0;
            lf.escapement = (int)(rot*10);
            lf.orientation = 0;
            lf.weight = (face == 2  ||  face == 4) ?
                eFontWeight_bold : eFontWeight_normal;
            lf.italic = (face == 3  ||  face == 4) ? 1 : 0;
            lf.underline = 0;
            lf.strikeOut = 0;
            lf.charSet = eDEFAULT_CHARSET;
            lf.outPrecision = eOUT_STROKE_PRECIS;
            lf.clipPrecision = eCLIP_DEFAULT_PRECIS;
            lf.quality = eANTIALIASED_QUALITY;
            lf.pitchAndFamily = eFF_DONTCARE + eDEFAULT_PITCH;
            lf.SetFace(familyUTF16);
        }
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o) << lf.height << lf.width
              << lf.escapement << lf.orientation << lf.weight
              << lf.italic << lf.underline << lf.strikeOut
              << lf.charSet << lf.outPrecision << lf.clipPrecision
              << lf.quality << lf.pitchAndFamily;
            o.append(lf.faceName, 64);
            //a Microsoft security update issued on 11 June 2019 no longer supports viewing EMF files with a LogFont object here (even though this is still allowed by the EMF design spec); below fills out the more complicated LogFontExDv which still works for viewing on Windows.
            o.append(128+64+64, '\0');//pad out to fill LogFontEx object
            o << TUInt4(0x08007664) // DesignVector signature
              << TUInt4(0);//specify no elements in design vector
            return o;
	}
    };

    struct SPoly : SRecord { //also == POLYLINE or POLYGON
        SRect  bounds;
        unsigned int count;
        SPoint *points;
        SPoly(ERecordType iType, int n, double *x, double *y) :
        SRecord(iType), points(new SPoint[n]) {
            bounds.Set((int) floor(x[0] + 0.5), (int) floor(y[0] + 0.5),
                       (int) floor(x[0] + 0.5), (int) floor(y[0] + 0.5));
            count = n;
            for (int i = 0;  i < n;  ++i) {
                points[i].Set((int) floor(x[i] + 0.5), (int) floor(y[i] + 0.5));
                if (points[i].x < bounds.left)   { bounds.left = points[i].x; }
                if (points[i].x > bounds.right)  { bounds.right = points[i].x; }
                if (points[i].y > bounds.bottom) { bounds.bottom = points[i].y;}
                if (points[i].y < bounds.top)    { bounds.top = points[i].y; }
            }
        }
        ~SPoly(void) { delete[] points; }
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << bounds << TUInt4(count);
            for (unsigned int i = 0;  i < count;  ++i) {
                o << points[i];
            }
            return o;
	}
    };

    struct S_SETTEXTALIGN : SRecord {
        TUInt4 mode;
        S_SETTEXTALIGN(void) : SRecord(eEMR_SETTEXTALIGN) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << mode;
        }
    };

    struct S_SETTEXTCOLOR : SRecord {
        SColorRef color;
        S_SETTEXTCOLOR(void) : SRecord(eEMR_SETTEXTCOLOR) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << color;
        }
    };

    struct S_SAVEDC : SRecord {
        S_SAVEDC(void) : SRecord(eEMR_SAVEDC) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct S_RESTOREDC : SRecord {
        TInt4 which;
        S_RESTOREDC(int w=-1) : SRecord(eEMR_RESTOREDC) {which=w;}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << which;
        }
    };

    struct S_SETWORLDTRANSFORM : SRecord {
        SXForm xform;
        S_SETWORLDTRANSFORM(void) : SRecord(eEMR_SETWORLDTRANSFORM) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << xform;
        }
    };

    struct S_MODIFYWORLDTRANSFORM : SRecord {
        SXForm xform;
        TUInt4 mode;
        S_MODIFYWORLDTRANSFORM(EModifyWorldTransformMode mwtm) :
        SRecord(eEMR_MODIFYWORLDTRANSFORM) { mode = mwtm;}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << xform << mode;
        }
    };

    struct S_SELECTOBJECT : SRecord {
        TUInt4 ihObject;
        S_SELECTOBJECT(void) : SRecord(eEMR_SELECTOBJECT) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << ihObject;
        }
    };

    struct S_SETBKMODE : SRecord {
        TUInt4 mode;
        S_SETBKMODE(void) : SRecord(eEMR_SETBKMODE) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << mode;
        }
    };

    struct S_SETPOLYFILLMODE : SRecord {
        TUInt4 mode;
        S_SETPOLYFILLMODE(void) : SRecord(eEMR_SETPOLYFILLMODE) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << mode;
        }
    };

    struct S_SETMAPMODE : SRecord {
        TUInt4 mode;
        S_SETMAPMODE(void) : SRecord(eEMR_SETMAPMODE) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << mode;
        }
    };

    struct S_SETMITERLIMIT : SRecord {
        TUInt4 miterLimit;
        S_SETMITERLIMIT(void) : SRecord(eEMR_SETMITERLIMIT) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << miterLimit;
        }
    };

    struct S_EOF : SRecord {
        TUInt4 nPalEntries;
        TUInt4 offPalEntries;
        TUInt4 nSizeLast;
        S_EOF(void) : SRecord(eEMR_EOF) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << nPalEntries << offPalEntries
                                         << nSizeLast;
        }
    };

    struct S_RECTANGLE : SRecord {
        SRect box;
        S_RECTANGLE(void) : SRecord(eEMR_RECTANGLE) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << box;
        }
    };

    struct S_ELLIPSE : SRecord {
        SRect box;
        S_ELLIPSE(void) : SRecord(eEMR_ELLIPSE) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << box;
        }
    };

    struct S_EXTSELECTCLIPRGN : SRecord {
        //NOT FULLY IMPLEMENTED (JUST RESETS TO DEFAULT)
        S_EXTSELECTCLIPRGN(void) : SRecord(eEMR_EXTSELECTCLIPRGN) {}
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << TUInt4(0) << TUInt4(5);
        }
    };

    struct S_INTERSECTCLIPRECT : SRecord {
        SRect rect;
        S_INTERSECTCLIPRECT(double x0, double y0, double x1, double y1) :
            SRecord(eEMR_INTERSECTCLIPRECT) { rect.Set(x0,y0,x1,y1); }
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << rect;
        }
    };

    struct S_SETSTRETCHBLTMODE : SRecord {
        TUInt4 mode;
        S_SETSTRETCHBLTMODE(int m) : SRecord(eEMR_SETSTRETCHBLTMODE) {
            mode = m;
        }
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << mode;
        }
    };

    struct S_SETBRUSHORGEX : SRecord {
        SPoint origin;
    S_SETBRUSHORGEX(int x, int y) : SRecord(eEMR_SETBRUSHORGEX) {
            origin.Set(x,y);
        }
	std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << origin;
        }
    };

    struct S_BITBLT : SRecord {
        struct SBitmapHeader {
            TUInt4 size;
            TInt4 width, height;
            TUInt2 planes;
            TUInt2 bitCount;
            TUInt4 compression;
            TUInt4 imageSize;
            TInt4 xPelsPerMeter;
            TInt4 yPelsPerMeter;
            TUInt4 colorUsed;
            TUInt4 colorImportant;
        };
        SRect bounds;
        TInt4 xDest, yDest;
        TInt4 cxDest,cyDest;
        TUInt4 bitBltRasterOp;
        TInt4 xSrc, ySrc;
        SXForm xformSrc;
        SColorRef bkColorSrc;
        TUInt4 usageSrc;
        int offBmiSrc, cbBmiSrc;
        int offBitsSrc, cbBitsSrc;
        SBitmapHeader bmpHead;
        std::string bmpData;
        S_BITBLT(unsigned int *data, unsigned int srcW, unsigned int srcH,
                 double x, double y, double w, double h) :
            SRecord(eEMR_BITBLT) {
            bounds.Set(x,x+w,y,y+h);
            xDest = x;
            yDest = y;
            xSrc = ySrc = 0;
            offBmiSrc = 25*4;//offset(S_BITBLT,bmp)
            cbBmiSrc = 10*4; //size of bitmap header
            offBitsSrc = offBmiSrc + cbBmiSrc;
            cbBitsSrc = srcW*srcH*4;//size of bitmap
            usageSrc = 0; // DIB_RGB_COLORS
            bitBltRasterOp = 0xCC0020; //SRCCOPY
            xformSrc.Set(1,0,0,1,0,0); // identity
            bkColorSrc.Set(0,0,0); //src bg color (irrelevant for us)
            cxDest = w;
            cyDest = h;
            bmpHead.size = cbBmiSrc;
            bmpHead.width = srcW;
            bmpHead.height = -srcH;
            bmpHead.planes = 1;
            bmpHead.bitCount = 0x20;
            bmpHead.compression = 0;//BI_RGB
            bmpHead.imageSize = 0; //ignored for BI_RGB
            bmpHead.xPelsPerMeter = 1; //arb?
            bmpHead.yPelsPerMeter = 1;
            bmpHead.colorUsed = 0;
            bmpHead.colorImportant = 0;
            bmpData.resize(srcW*srcH*4);
            for (unsigned int i = 0;  i < srcW*srcH; ++i) {
                bmpData[4*i+0] = R_BLUE(data[i]);
                bmpData[4*i+1] = R_GREEN(data[i]);
                bmpData[4*i+2] = R_RED(data[i]);
                bmpData[4*i+3] = R_ALPHA(data[i]);
            }
        }
	std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << bounds << xDest << yDest <<
                cxDest << cyDest << bitBltRasterOp << xSrc << ySrc <<
                xformSrc << bkColorSrc << usageSrc << TUInt4(offBmiSrc) <<
                TUInt4(cbBmiSrc) << TUInt4(offBitsSrc) << TUInt4(cbBitsSrc) <<
                bmpHead.size << bmpHead.width << bmpHead.height <<
                bmpHead.planes << bmpHead.bitCount << bmpHead.compression <<
                bmpHead.imageSize << bmpHead.xPelsPerMeter <<
                bmpHead.yPelsPerMeter << bmpHead.colorUsed <<
                bmpHead.colorImportant;
            o.append(bmpData);
            return o;
        }
    };

    struct S_STRETCHBLT : SRecord {
        struct SBitmapHeader {
            TUInt4 size;
            TInt4 width, height;
            TUInt2 planes;
            TUInt2 bitCount;
            TUInt4 compression;
            TUInt4 imageSize;
            TInt4 xPelsPerMeter;
            TInt4 yPelsPerMeter;
            TUInt4 colorUsed;
            TUInt4 colorImportant;
        };
        SRect bounds;
        TInt4 xDest, yDest;
        TInt4 cxDest,cyDest;
        TUInt4 bitBltRasterOp;
        TInt4 xSrc, ySrc;
        SXForm xformSrc; //not in stretchdibits
        SColorRef bkColorSrc; //not in stretchdibits
        TUInt4 usageSrc;
        int offBmiSrc, cbBmiSrc;
        int offBitsSrc, cbBitsSrc;
        TInt4 cxSrc, cySrc;
        SBitmapHeader bmpHead;
        std::string bmpData;
        S_STRETCHBLT(unsigned int *data, unsigned int srcW, unsigned int srcH,
                     double x, double y, double w, double h) :
            SRecord(eEMR_STRETCHBLT) {
            bounds.Set(x,x+w,y,y+h);
            xDest = x;
            yDest = y;
            xSrc = ySrc = 0;
            cxSrc = srcW;
            cySrc = srcH;
            offBmiSrc = 27*4;//offset(S_STRETCHBLT,bmp)
            cbBmiSrc = 10*4; //size of bitmap header
            offBitsSrc = offBmiSrc + cbBmiSrc;
            cbBitsSrc = srcW*srcH*4;//size of bitmap
            usageSrc = 0; // DIB_RGB_COLORS
            bitBltRasterOp = 0xCC0020; //SRCCOPY
            xformSrc.Set(1,0,0,1,0,0); // identity
            bkColorSrc.Set(0,0,0); //src bg color (irrelevant for us)
            cxDest = w;
            cyDest = h;
            bmpHead.size = cbBmiSrc;
            bmpHead.width = srcW;
            bmpHead.height = -srcH;
            bmpHead.planes = 1;
            bmpHead.bitCount = 0x20;
            bmpHead.compression = 0;//BI_RGB
            bmpHead.imageSize = 0; //ignored for BI_RGB
            bmpHead.xPelsPerMeter = 1; //arb?
            bmpHead.yPelsPerMeter = 1;
            bmpHead.colorUsed = 0;
            bmpHead.colorImportant = 0;
            bmpData.resize(srcW*srcH*4);
            for (unsigned int i = 0;  i < srcW*srcH; ++i) {
                bmpData[4*i+0] = R_BLUE(data[i]);
                bmpData[4*i+1] = R_GREEN(data[i]);
                bmpData[4*i+2] = R_RED(data[i]);
                bmpData[4*i+3] = R_ALPHA(data[i]);
            }
        }
	std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << bounds << xDest << yDest <<
                cxDest << cyDest << bitBltRasterOp << xSrc << ySrc <<
                xformSrc << bkColorSrc << usageSrc << TUInt4(offBmiSrc) <<
                TUInt4(cbBmiSrc) << TUInt4(offBitsSrc) << TUInt4(cbBitsSrc) <<
                cxSrc << cySrc <<                 
                bmpHead.size << bmpHead.width << bmpHead.height <<
                bmpHead.planes << bmpHead.bitCount << bmpHead.compression <<
                bmpHead.imageSize << bmpHead.xPelsPerMeter <<
                bmpHead.yPelsPerMeter << bmpHead.colorUsed <<
                bmpHead.colorImportant;
            o.append(bmpData);
            return o;
        }
    };

    struct ObjectPtrCmp {
        bool operator() (SObject* o1, SObject* o2) const {
            if (o1->iType < o2->iType) {
                return true;
            } else if (o1->iType > o2->iType) {
                return false;
            } else { //same type -- so compare details
                switch (o1->iType) {
                case eEMR_EXTCREATEPEN: {
                    SPen *p1 = dynamic_cast<SPen*>(o1);
                    SPen *p2 = dynamic_cast<SPen*>(o2);
                    int res = memcmp(&p1->elp, &p2->elp, sizeof(p1->elp));
                    if (res != 0) return res < 0;
                    if (p1->elp.numEntries < p2->elp.numEntries) return true;
                    if (p1->elp.numEntries > p2->elp.numEntries) return false;
                    return p1->styleEntries < p2->styleEntries;
                }
                case eEMR_CREATEBRUSHINDIRECT:
                    return memcmp(&dynamic_cast<SBrush*>(o1)->lb,
                                  &dynamic_cast<SBrush*>(o2)->lb,
                                  sizeof(dynamic_cast<SBrush*>(o1)->lb)) < 0;
                case eEMR_EXTCREATEFONTINDIRECTW:
                    return memcmp(&dynamic_cast<SFont*>(o1)->lf,
                                  &dynamic_cast<SFont*>(o2)->lf,
                                  sizeof(dynamic_cast<SFont*>(o1)->lf)) < 0;
                default: {//should never happen
                    throw std::logic_error("EMF object table scrambled");
                }
                }
            }
        }
    };

    class CObjectTable {
    public:
        CObjectTable(void) {
            for (unsigned int i = 0;  i < eEMR_last;  ++i) {
                m_CurrObj[i] = -1;
            }
            m_CurrMiterLimit = -1;
        }
        ~CObjectTable(void) {
            for (TIndex::iterator i = m_Objects.begin();
                 i != m_Objects.end();  ++i) {
                delete *i;
            }
        }
        unsigned int GetSize(void) const { return m_Objects.size(); }

        unsigned char GetPen(unsigned int col, double lwd, unsigned int lty,
                             unsigned int lend, unsigned int ljoin,
                             unsigned int lmitre, double ps2dev,
                             bool useUserLty, EMF::ofstream &out) {
            SPen *pen = new SPen(col, lwd, lty, lend, ljoin, ps2dev,
                                 useUserLty);
            if (ljoin == GE_MITRE_JOIN  &&
                (int) lmitre != m_CurrMiterLimit) {
                S_SETMITERLIMIT emr;
                emr.miterLimit = lmitre;
                emr.Write(out);
                m_CurrMiterLimit = lmitre;
            }
            return x_SelectObject(pen, out)->m_ObjId;
        }
        unsigned char GetBrush(unsigned int col, EMF::ofstream &out) {
            SBrush *brush = new SBrush(col);
            return x_SelectObject(brush, out)->m_ObjId;
        }
        unsigned char GetFont(unsigned char face, int size,
                              const std::string &familyUTF16,
                              double rot,
                              EMF::ofstream &out) {
            SFont *font = new SFont(face, size, familyUTF16, rot);
            return x_SelectObject(font, out)->m_ObjId;
        }
    private:
        SObject* x_GetObject(SObject *obj, EMF::ofstream &out) {
            TIndex::iterator i = m_Objects.find(obj);
            if (i == m_Objects.end()) {
                i = m_Objects.insert(obj).first;
                obj->m_ObjId = m_Objects.size();
                obj->Write(out);
            } else {
                delete obj;
            }
            return (*i);
        }
        SObject* x_SelectObject(SObject *obj, EMF::ofstream &out) {
            obj = x_GetObject(obj, out);
            if (m_CurrObj[obj->iType] != (int)obj->m_ObjId) {
                S_SELECTOBJECT emr;
                emr.ihObject = obj->m_ObjId;
                emr.Write(out);
                m_CurrObj[obj->iType] = obj->m_ObjId;
            }
            return obj;
        }
    private:
        typedef std::set<SObject*, ObjectPtrCmp> TIndex;        
        TIndex m_Objects;
        int m_CurrObj[eEMR_last];
        int m_CurrMiterLimit;
    };

} //end of EMF namespace

#endif //EMF__H
